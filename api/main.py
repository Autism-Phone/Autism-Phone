from fastapi import FastAPI, HTTPException, BackgroundTasks
from pydantic import BaseModel
import uuid
import mysql.connector
from datetime import datetime, timedelta
import json
import logging

app = FastAPI()

# Konfiguracja logowania i DB
logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

DB_CONFIG = {
    "host": "localhost",
    "user": "autism",
    "password": "h4s10",
    "database": "game_sessions"  # Baza główna do śledzenia sesji
}

class GameCreate(BaseModel):
    max_players: int = 10

class PlayerJoin(BaseModel):
    game_code: str
    name: str

class Submission(BaseModel):
    player_id: str
    content: dict  # Może zawierać 'text' lub 'drawing'

@app.post("/create-game")
async def create_game(bg_tasks: BackgroundTasks):
    game_id = str(uuid.uuid4())
    invite_code = str(uuid.uuid4())[:8].upper()
    
    try:
        # Utwórz nową bazę dla gry
        conn = mysql.connector.connect(**DB_CONFIG)
        cursor = conn.cursor()
        cursor.execute(f"CREATE DATABASE `game_{game_id}`")
        
        # Inicjalizacja struktury bazy
        init_db(game_id)
        
        # Zapisz metadane gry
        cursor.execute("""
            INSERT INTO games 
            (id, invite_code, status, created_at) 
            VALUES (%s, %s, %s, %s)
        """, (game_id, invite_code, 'waiting', datetime.now()))
        
        conn.commit()
        return {"game_id": game_id, "invite_code": invite_code}
    
    except Exception as e:
        logger.error(f"Error creating game: {e}")
        raise HTTPException(status_code=500, detail="Game creation failed")

def init_db(game_id: str):
    # Utwórz połączenie z nową bazą gry
    conn = get_game_db(game_id)
    cursor = conn.cursor()


    try:
        # Tabela players (dla konkretnej gry)
        cursor.execute("""
            CREATE TABLE players (
                id VARCHAR(36) PRIMARY KEY,
                name VARCHAR(255) NOT NULL,
                assignment_order JSON,
                is_active BOOLEAN DEFAULT TRUE,
                created_at DATETIME DEFAULT CURRENT_TIMESTAMP
            ) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci
        """)

        # Tabela rounds
        cursor.execute("""
            CREATE TABLE rounds (
                id VARCHAR(36) PRIMARY KEY,
                number INT NOT NULL,
                type ENUM('text', 'drawing') NOT NULL,
                start_time DATETIME NOT NULL,
                end_time DATETIME NOT NULL
            ) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci
        """)

        # Tabela submissions z kluczami obcymi
        cursor.execute("""
            CREATE TABLE submissions (
                id VARCHAR(36) PRIMARY KEY,
                player_id VARCHAR(36) NOT NULL,
                round_id VARCHAR(36) NOT NULL,
                content JSON NOT NULL,
                FOREIGN KEY (player_id) 
                    REFERENCES players(id)
                    ON DELETE CASCADE,
                FOREIGN KEY (round_id) 
                    REFERENCES rounds(id)
                    ON DELETE CASCADE,
                INDEX idx_player (player_id),
                INDEX idx_round (round_id)
            ) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci
        """)

        conn.commit()
        logging.info(f"Database game_{game_id} initialized successfully")

    except mysql.connector.Error as err:
        logging.error(f"Database initialization failed: {err}")
        raise
    finally:
        cursor.close()
        conn.close()

@app.post("/join-game")
async def join_game(request: PlayerJoin):
    # Połączenie z główną bazą danych
    main_conn = mysql.connector.connect(**DB_CONFIG)
    main_cursor = main_conn.cursor(dictionary=True)
    
    # Krok 1: Znajdź grę po kodzie
    main_cursor.execute("""
        SELECT * FROM games 
        WHERE invite_code = %s 
        AND status = 'waiting'
    """, (request.game_code,))
    
    game = main_cursor.fetchone()
    if not game:
        main_cursor.close()
        main_conn.close()
        raise HTTPException(status_code=404, detail="Game not found")
    
    # Krok 2: Sprawdź unikalność nicku w bazie gry
    game_conn = get_game_db(game['id'])
    game_cursor = game_conn.cursor()
    
    try:
        # Sprawdź czy nick już istnieje
        game_cursor.execute("""
            SELECT COUNT(*) AS count 
            FROM players 
            WHERE name = %s 
        """, (request.name,))
        temp = game_cursor.fetchone()
        name_count = temp[0]
        
        if name_count > 0:
            raise HTTPException(
                status_code=400, 
                detail="Player with this name already exists in the game"
            )
        
        # Krok 3: Dodaj gracza do game_players (główna baza)
        player_id = str(uuid.uuid4())
        main_cursor.execute("""
            INSERT INTO game_players 
            (game_id, player_id) 
            VALUES (%s, %s)
        """, (game['id'], player_id))
        
        # Krok 4: Dodaj gracza do players (baza gry)
        game_cursor.execute("""
            INSERT INTO players 
            (id, name) 
            VALUES (%s, %s)
        """, (player_id, request.name))
        
        main_conn.commit()
        game_conn.commit()
        
        return {"player_id": player_id, "game_id": game['id']}
        
    except mysql.connector.Error as err:
        main_conn.rollback()
        game_conn.rollback()
        logger.error(f"Database error: {err}")
        raise HTTPException(status_code=500, detail="Database operation failed")
        
    finally:
        main_cursor.close()
        main_conn.close()
        game_cursor.close()
        game_conn.close()

@app.get("/game-state/{game_id}")
async def get_game_state(game_id: str, player_id: str):
    conn = get_game_db(game_id)
    cursor = conn.cursor(dictionary=True)
    
    # Pobierz aktualną rundę
    cursor.execute("""
        SELECT * FROM rounds 
        ORDER BY number DESC LIMIT 1
    """)
    current_round = cursor.fetchone()
    
    # Sprawdź stan gracza
    cursor.execute("""
        SELECT * FROM submissions 
        WHERE player_id = %s 
        AND round_id = %s
    """, (player_id, current_round['id']))
    
    submission = cursor.fetchone()
    
    return {
        "round": current_round,
        "time_left": (current_round['end_time'] - datetime.now()).total_seconds(),
        "submitted": bool(submission)
    }

@app.post("/submit")
async def submit(submission: Submission):
    conn = get_game_db_by_player(submission.player_id)
    cursor = conn.cursor()
    
    try:
        cursor.execute("""
            INSERT INTO submissions 
            (id, player_id, round_id, content) 
            VALUES (%s, %s, %s, %s)
        """, (str(uuid.uuid4()), submission.player_id, get_current_round(), json.dumps(submission.content)))
        
        conn.commit()
        return {"status": "success"}
    
    except Exception as e:
        logger.error(f"Submission error: {e}")
        raise HTTPException(status_code=400, detail="Submission failed")

def get_game_db(game_id: str):
    return mysql.connector.connect(
        host=DB_CONFIG['host'],
        user=DB_CONFIG['user'],
        password=DB_CONFIG['password'],
        database=f"game_{game_id}"
    )