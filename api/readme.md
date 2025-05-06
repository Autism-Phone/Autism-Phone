# Autism Phone API Documentation

## Table of Contents
1. [Database Setup](#database-setup)
2. [API Endpoints](#api-endpoints)
3. [Game Flow](#game-flow)
4. [Data Structures](#data-structures)
5. [Security Features](#security-features)
6. [Error Codes](#error-codes)
7. [Examples](#examples)
8. [Frontend Implementation](#frontend-implementation)

## Database Setup
```sql
-- Create user and privileges
CREATE USER 'autism'@'localhost' IDENTIFIED BY 'h4s10';
GRANT ALL PRIVILEGES ON *.* TO 'autism'@'localhost';

-- Main database setup
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

## API Endpoints

### 1. Create Game
```http
POST /create-game
```
**Response:**
```json
{
  "game_id": "550e8400-e29b-41d4-a716-446655440000",
  "invite_code": "ABCD1234"
}
```

### 2. Join Game
```http
POST /join-game
{
  "invite_code": "ABCD1234",
  "name": "Player1"
}
```
**Response:**
```json
{
  "player_id": "d9b6f1f0-ae3a-4b0c-9c4a-5a5d5f5e5d5c",
  "game_id": "550e8400-e29b-41d4-a716-446655440000"
}
```

### 3. Start Game
```http
POST /start-game/{game_id}
```

### 4. Get Game State
```http
GET /game-state/{game_id}?player_id={player_id}
```

### 5. Submit Answer
```http
POST /submit
{
  "player_id": "d9b6f1f0-ae3a-4b0c-9c4a-5a5d5f5e5d5c",
  "content": {
    "text": "sample text",
    "drawing": [[255,255,255], [0,0,0], ...]
  }
}
```

## Game Flow
1. **Initialization**: Host creates game → gets game_id and invite_code
2. **Joining**: Players join with invite code and unique name
3. **Rounds**:
   - Alternate between text (20s) and drawing (40s) rounds
   - Number of rounds = number of players
4. **Prompts**:
   - Text rounds: Receive previous drawing as RGB array
   - Drawing rounds: Receive previous text description
5. **Completion**: Show transformation chain, automatic cleanup

## Data Structures

### Round Object
```typescript
interface Round {
  number: number;
  type: 'text' | 'drawing';
  time_left: number;
}
```

### Game State Responses
**Waiting State:**
```json
{
  "status": "waiting"
}
```

**Text Round with Image Prompt:**
```json
{
  "status": "in_progress",
  "round": {
    "number": 3,
    "type": "text",
    "time_left": 17.3
  },
  "submitted": false,
  "prompt": [
    [255,255,255], [243,214,178], 
    [0,0,0], ..., [127,127,127]
  ]
}
```

**Drawing Round with Text Prompt:**
```json
{
  "status": "in_progress",
  "round": {
    "number": 2,
    "type": "drawing",
    "time_left": 35.7
  },
  "submitted": false,
  "prompt": "A magical forest at twilight"
}
```

**Completed Game:**
```json
{
  "status": "finished",
  "message": "Game has ended"
}
```

## Security Features
- 🔒 UUIDv4 authentication
- 🛡️ Parameterized SQL queries
- 🗄️ Isolated game databases
- 🔄 Automatic database cleanup
- 📏 Strict input validation:
  - Text: 500 character limit
  - Drawing: 800,000 RGB values

## Error Codes
| Code | Error Message                 | Description                     |
|------|-------------------------------|---------------------------------|
| 400  | Invalid content format        | Malformed JSON                 |
| 401  | Invalid player/game ID        | UUID verification failed       |
| 403  | Submission too late           | Round timer expired            |
| 404  | Game not found                | Invalid game_id                |
| 409  | Name already exists           | Duplicate player name          |
| 413  | Content too large             | Exceeds size limits            |
| 422  | Invalid drawing format        | Array dimension mismatch       |
| 500  | Database operation failed     | Connection/query issues        |

