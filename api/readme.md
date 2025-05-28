
# Gra "Autism Phone" API Documentation

### Tworzenie bazy danych
```sql
create user 'autism'@localhost identified by 'h4s10';

GRANT ALL PRIVILEGES ON *.* TO 'autism'@localhost IDENTIFIED BY 'h4s10';
CREATE DATABASE IF NOT EXISTS game_sessions;

USE game_sessions;

CREATE TABLE games (
    id VARCHAR(36) PRIMARY KEY,
    invite_code VARCHAR(8) UNIQUE,
    status ENUM('waiting', 'in_progress', 'finished') DEFAULT 'waiting',
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    max_players INT DEFAULT 10
);
CREATE TABLE game_players (
    game_id VARCHAR(36) NOT NULL,
    player_id VARCHAR(36) NOT NULL,
    PRIMARY KEY (game_id, player_id),
    FOREIGN KEY (game_id) REFERENCES games(id) ON DELETE CASCADE
);
```
### 0. Uruchomienie API
```bash
uvicorn main:app --port 2137 --host 0.0.0.0
```

### 1. Tworzenie nowej gry
**Request:**
```http
POST /create-game
```

**Response (success):**
```json
{
  "game_id": "550e8400-e29b-41d4-a716-446655440000",
  "invite_code": "ABCD1234"
}
```

### 2. Dołączanie do gry
**Request:**
```http
POST /join-game
{
  "invite_code": "ABCD1234",
  "name": "Player1"
}
```

**Response (success):**
```json
{
  "player_id": "d9b6f1f0-ae3a-4b0c-9c4a-5a5d5f5e5d5c",
  "game_id": "550e8400-e29b-41d4-a716-446655440000"
}
```

**Możliwe błędy:**
- 404 Game not found
- 400 Player name already exists
- 500 Database error

### 3. Sprawdzanie stanu gry
**Request:**
```http
GET /game-state/{game_id}?player_id={player_id}
```

**Przykładowe odpowiedzi:**

a) Gra nie rozpoczęta:
```json
{
  "status": "waiting"
}
```

b) Gra w trakcie:
```json
{
  "status": "in_progress",
  "round": {
    "number": 2,
    "type": "drawing",
    "time_left": 35.7
  },
  "submitted": false
}
```

c) Gra zakończona:
```json
{
  "status": "finished",
  "message": "Game has ended"
}
```

### 4. Wysyłanie odpowiedzi
**Request:**
```http
POST /submit
{
  "player_id": "d9b6f1f0-ae3a-4b0c-9c4a-5a5d5f5e5d5c",
  "content": {
    "text": "latający słoń"
  }
}
```

**Formaty content:**
- Dla rundy tekstowej:
  ```json
  {"text": "dowolny ciąg znaków"}
  ```
- Dla rundy rysunkowej:
  ```json
  {"drawing": "coś w base64"}  # obrazek
  ```

**Response (success):**
```json
{"status": "success"}
```

## Przebieg gry

1. **Inicjalizacja:**
   - Host tworzy grę i otrzymuje kod zaproszenia
   - Gracze dołączają podając kod i unikalną nazwę

2. **Rozgrywka:**
   - Automatyczny start przy min. 2 graczach
   - Rund na przemian tekstowe i rysunkowe
   - Czas na turę: 20s dla tekstu, 40s dla rysunku
   - Liczba rund = liczba graczy

3. **Mechanika:**
   - Każdy gracz otrzymuje wyniki poprzedniej rundy
   - Losowe przydzielanie zadań między graczami
   - Automatyczna detekcja nieaktywnych graczy

4. **Zakończenie:**
   - Pokazanie pełnego łańcucha skojarzeń
   - Automatyczne czyszczenie danych gry

## Struktury danych

### Runda:
```typescript
interface Round {
  number: number;
  type: 'text' | 'drawing';
  time_left: number;
}
```

### Historia gry:
```typescript
interface GameHistory {
  author: string;
  round_number: number;
  round_type: 'text' | 'drawing';
  content: object;
}
```

## Bezpieczeństwo
- Wszystkie ID w formacie UUIDv4
- Parametryzowane zapytania SQL
- Każda gra w osobnej bazie danych
- Autentykacja przez player_id i game_id
- Walidacja danych wejściowych przez Pydantic

