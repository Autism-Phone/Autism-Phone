from fastapi import FastAPI, HTTPException, BackgroundTasks
from pydantic import BaseModel
import uuid
import mysql.connector
from datetime import datetime, timedelta
import json
import logging
import asyncio

app = FastAPI()

# Konfiguracja logowania i DB
logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

DB_CONFIG = {
    "host": "localhost",
    "user": "autism",
    "password": "h4s10",
    "database": "game_sessions"
}

class GameCreate(BaseModel):
    max_players: int = 10

class PlayerJoin(BaseModel):
    invite_code: str
    name: str

class Submission(BaseModel):
    player_id: str
    content: dict

# Nowa klasa pomocnicza
class RoundInfo(BaseModel):
    round_number: int
    round_type: str
    start_time: datetime
    end_time: datetime

@app.post("/create-game")
async def create_game(bg_tasks: BackgroundTasks):
    game_id = str(uuid.uuid4())
    invite_code = str(uuid.uuid4())[:8].upper()
    
    try:
        conn = mysql.connector.connect(**DB_CONFIG)
        cursor = conn.cursor()
        cursor.execute(f"CREATE DATABASE `game_{game_id}`")
        
        init_db(game_id)
        
        cursor.execute("""
            INSERT INTO games 
            (id, invite_code, status, created_at) 
            VALUES (%s, %s, %s, %s)
        """, (game_id, invite_code, 'waiting', datetime.now()))
        
        conn.commit()
        bg_tasks.add_task(game_loop, game_id)
        return {"game_id": game_id, "invite_code": invite_code}
    
    except Exception as e:
        logger.error(f"Error creating game: {e}")
        raise HTTPException(status_code=500, detail="Game creation failed")
    finally:
        cursor.close()
        conn.close()

def init_db(game_id: str):
    conn = get_game_db(game_id)
    cursor = conn.cursor()

    try:
        cursor.execute("""
            CREATE TABLE players (
                id VARCHAR(36) PRIMARY KEY,
                name VARCHAR(255) NOT NULL,
                assignment_order JSON,
                is_active BOOLEAN DEFAULT TRUE,
                created_at DATETIME DEFAULT CURRENT_TIMESTAMP
            ) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci
        """)

        cursor.execute("""
            CREATE TABLE rounds (
                id VARCHAR(36) PRIMARY KEY,
                number INT NOT NULL,
                type ENUM('text', 'drawing') NOT NULL,
                start_time DATETIME NOT NULL,
                end_time DATETIME NOT NULL
            ) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci
        """)

        cursor.execute("""
            CREATE TABLE submissions (
                id VARCHAR(36) PRIMARY KEY,
                player_id VARCHAR(36) NOT NULL,
                round_id VARCHAR(36) NOT NULL,
                content JSON NOT NULL,
                FOREIGN KEY (player_id) REFERENCES players(id) ON DELETE CASCADE,
                FOREIGN KEY (round_id) REFERENCES rounds(id) ON DELETE CASCADE,
                INDEX idx_player (player_id),
                INDEX idx_round (round_id)
            ) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci
        """)

        conn.commit()
    except mysql.connector.Error as err:
        logger.error(f"Database initialization failed: {err}")
        raise
    finally:
        cursor.close()
        conn.close()


@app.post("/start-game/{game_id}")
async def start_game(game_id: str):
    try:
        conn = mysql.connector.connect(**DB_CONFIG)
        cursor = conn.cursor(dictionary=True)
        
        # 1. Pobierz aktualny status z blokadą
        cursor.execute("SELECT status FROM games WHERE id = %s FOR UPDATE", (game_id,))
        result = cursor.fetchone()
        
        if not result:
            raise HTTPException(status_code=404, detail="Game not found")
            
        current_status = result['status']
        
        if current_status != 'waiting':
            raise HTTPException(
                status_code=400,
                detail=f"Cannot start game in {current_status} status"
            )
        
        # 2. Aktualizacja statusu z potwierdzeniem
        cursor.execute(
            "UPDATE games SET status = 'in_progress' WHERE id = %s",
            (game_id,)
        )
        conn.commit()
        
        # 3. Weryfikacja zapisu
        cursor.execute("SELECT status FROM games WHERE id = %s", (game_id,))
        updated_status = cursor.fetchone()['status']
        logger.info(f"Game {game_id} new status: {updated_status}")
        return {"status": "success", "message": f"Game {game_id} new status: {updated_status}"}
        
        #return {"status": "success", "message": "Game started successfully"}
        
    except mysql.connector.Error as err:
        logger.error(f"Database error: {err}")
        raise HTTPException(500, detail="Database operation failed")
    finally:
        cursor.close()
        conn.close()


async def game_loop(game_id: str):
    logger.info(f"Starting game loop for {game_id}")
    game_conn = get_game_db(game_id)
    
    main_conn = mysql.connector.connect(
        **DB_CONFIG,
        isolation_level='READ COMMITTED',
        autocommit=True
    )
    
    try:
        # Szybsze sprawdzanie co 300ms
        while True:
            with main_conn.cursor() as cursor:
                cursor.execute(
                    "SELECT status FROM games WHERE id = %s",
                    (game_id,)
                )
                result = cursor.fetchone()
                
                if result and result[0] == 'in_progress':
                    logger.info(f"Game {game_id} status confirmed!")
                    break
                    
            await asyncio.sleep(0.3)
        
        
        logger.info(f"Game {game_id} officially started!")
        
        # Reszta oryginalnej logiki
        with game_conn.cursor() as cursor:
            cursor.execute("SELECT COUNT(*) FROM players")
            total_rounds = cursor.fetchone()[0]

        
        # Main rounds loop
        for round_number in range(1, total_rounds + 1):
            round_type = 'text' if round_number % 2 == 1 else 'drawing'
            duration = 20 if round_type == 'text' else 40
            
            # Create new round
            round_id = str(uuid.uuid4())
            start_time = datetime.now()
            end_time = start_time + timedelta(seconds=duration)
            
            with game_conn.cursor() as cursor:
                cursor.execute("""
                    INSERT INTO rounds 
                    (id, number, type, start_time, end_time)
                    VALUES (%s, %s, %s, %s, %s)
                """, (round_id, round_number, round_type, start_time, end_time))
                game_conn.commit()
            
            logger.info(f"Started round {round_number} ({round_type}) for game {game_id}")
            
            # Wait for round duration
            await asyncio.sleep(duration)
            with game_conn.cursor() as cursor:
                        # W game_loop po await asyncio.sleep(duration):
                cursor.execute("SELECT COUNT(*) FROM submissions WHERE round_id = %s", (round_id,))
                submissions_count = cursor.fetchone()[0]
                if submissions_count < total_players:
                    logger.warning(f"Not all players submitted in round {round_number}")
        
        # Finalize game
        with main_conn.cursor() as cursor:
            cursor.execute("UPDATE games SET status = 'finished' WHERE id = %s", (game_id,))
            main_conn.commit()
        
    except Exception as e:
        logger.error(f"Game loop error: {e}")
        with main_conn.cursor() as cursor:
            cursor.execute(
                "UPDATE games SET status = 'errored' WHERE id = %s",
                (game_id,)
            )
            main_conn.commit()
    finally:
        game_conn.close()
        main_conn.close()

@app.post("/join-game")
async def join_game(request: PlayerJoin):
    main_conn = mysql.connector.connect(**DB_CONFIG)
    main_cursor = main_conn.cursor(dictionary=True)
    
    try:
        main_cursor.execute("SELECT * FROM games WHERE invite_code = %s AND status = 'waiting'", (request.invite_code,))
        game = main_cursor.fetchone()
        print("game", game)
        if not game:
            raise HTTPException(404, "Game not found")
        
        game_conn = get_game_db(game['id'])
        with game_conn.cursor() as game_cursor:
            game_cursor.execute("SELECT COUNT(*) FROM players WHERE name = %s", (request.name,))
            if game_cursor.fetchone()[0] > 0:
                raise HTTPException(400, "Player name already exists")
            
            player_id = str(uuid.uuid4())
            main_cursor.execute("INSERT INTO game_players (game_id, player_id) VALUES (%s, %s)", (game['id'], player_id))
            game_cursor.execute("INSERT INTO players (id, name) VALUES (%s, %s)", (player_id, request.name))
            
            main_conn.commit()
            game_conn.commit()
            game_cursor.execute("SELECT COUNT(*) FROM players")
            res=game_cursor.fetchone()
            print("po dodaniu",res)
            game_cursor.close()
            main_cursor.close()
            game_conn.close()
            main_conn.close()
            return {"player_id": player_id, "game_id": game['id']}
            
    except mysql.connector.Error as err:
        logger.error(f"Database error: {err}")
        raise HTTPException(500, "Database operation failed")
    finally:
        main_cursor.close()
        main_conn.close()
        if 'game_conn' in locals():
            game_conn.close()

@app.get("/game-state/{game_id}")
async def get_game_state(game_id: str, player_id: str):
    try:
        # 1. Sprawdź główny status gry z blokadą
        with mysql.connector.connect(**DB_CONFIG, autocommit=True) as main_conn:
            with main_conn.cursor(dictionary=True) as main_cursor:
                main_cursor.execute(
                    "SELECT status FROM games WHERE id = %s FOR SHARE",
                    (game_id,)
                )
                game_info = main_cursor.fetchone()
                
                if not game_info:
                    raise HTTPException(404, detail="Game not found")
                
                main_status = game_info['status']

        # 2. Obsłuż różne statusy
        if main_status == 'finished':
            return {"status": "finished", "message": "Game has ended"}
            
        if main_status == 'errored':
            return {"status": "errored", "message": "Game encountered an error"}

        if main_status == 'waiting':
            return {"status": "waiting"}

        # 3. Sprawdź szczegóły dla gry w toku
        with get_game_db(game_id) as game_conn:
            with game_conn.cursor(dictionary=True) as game_cursor:
                # 3a. Sprawdź czy są jakiekolwiek rundy
                game_cursor.execute("SELECT 1 FROM rounds LIMIT 1")
                if not game_cursor.fetchone():
                    return {"status": "starting"}
                
                # 3b. Znajdź aktywną rundę
                game_cursor.execute("""
                    SELECT * FROM rounds 
                    WHERE end_time > NOW()
                    ORDER BY number ASC
                    LIMIT 1
                """)
                current_round = game_cursor.fetchone()
                
                # 3c. Jeśli brak aktywnej, weź ostatnią ukończoną
                if not current_round:
                    game_cursor.execute("""
                        SELECT * FROM rounds 
                        ORDER BY number DESC 
                        LIMIT 1
                    """)
                    current_round = game_cursor.fetchone()
                    if not current_round:
                        return {"status": "starting"}
                
                # 3d. Sprawdź zgłoszenie gracza
                game_cursor.execute("""
                    SELECT 1 FROM submissions 
                    WHERE player_id = %s AND round_id = %s
                """, (player_id, current_round['id']))
                submitted = bool(game_cursor.fetchone())

                # 3e. Oblicz pozostały czas
                time_left = (current_round['end_time'] - datetime.now()).total_seconds()
                time_left = max(0, round(time_left, 1))

                return {
                    "status": "in_progress",
                    "round": {
                        "number": current_round['number'],
                        "type": current_round['type'],
                        "time_left": time_left
                    },
                    "submitted": submitted
                }

    except HTTPException as he:
        raise he
    except mysql.connector.Error as err:
        logger.error(f"Database error: {err}")
        raise HTTPException(500, "Database operation failed")
    except Exception as e:
        logger.error(f"Game state error: {e}")
        raise HTTPException(500, "Could not retrieve game state")
    

@app.post("/submit")
async def submit(submission: Submission):
    try:
        with get_game_db_by_player(submission.player_id) as conn:
            with conn.cursor() as cursor:
                # Get current round
                cursor.execute("""
                    SELECT id FROM rounds 
                    WHERE end_time > NOW()
                    ORDER BY number ASC
                    LIMIT 1
                """)
                round_id = cursor.fetchone()
                if not round_id:
                    raise HTTPException(400, "No active round")
                
                # Insert submission
                cursor.execute("""
                    INSERT INTO submissions 
                    (id, player_id, round_id, content)
                    VALUES (%s, %s, %s, %s)
                """, (str(uuid.uuid4()), submission.player_id, round_id[0], json.dumps(submission.content)))
                
                conn.commit()
                return {"status": "success"}
    except mysql.connector.Error as err:
        logger.error(f"Submission error: {err}")
        raise HTTPException(500, "Database operation failed")

def get_game_db(game_id: str):
    return mysql.connector.connect(
        host=DB_CONFIG['host'],
        user=DB_CONFIG['user'],
        password=DB_CONFIG['password'],
        database=f"game_{game_id}"
    )

def get_game_db_by_player(player_id: str):
    with mysql.connector.connect(**DB_CONFIG) as conn:
        with conn.cursor() as cursor:
            cursor.execute("SELECT game_id FROM game_players WHERE player_id = %s", (player_id,))
            game_id = cursor.fetchone()
            if not game_id:
                raise HTTPException(404, "Player not found")
            return get_game_db(game_id[0])