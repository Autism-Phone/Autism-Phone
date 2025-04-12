### 1. Inicjalizacja gry
**Flow front-endu:**
```javascript
POST /create-game → 
   {game_id: "uuid", invite_code: "ABCD1234"}

Wyświetl kod zaproszenia u hosta
Czekaj na graczy (polling co 1s na /game-state)
```

### 2. Dołączanie graczy
**Flow dla każdego gracza:**
```javascript
POST /join-game {"game_code": "ABCD1234", "name": "Player1"} → 
   {player_id: "uuid", game_id: "uuid"}

Zapisz player_id i game_id w localStorage
Rozpocznij polling /game-state co 1 sekundę
```

### 3. Faza oczekiwania na start
**Przykładowa odpowiedź /game-state:**
```json
{
  "status": "waiting",
  "players": ["Player1", "Player2"],
  "start_time": null
}
```
Front-end wyświetla listę oczekujących graczy i odlicza czas do startu (automatyczny start po zebraniu min. 2 graczy)

### 4. Rozpoczęcie gry - Runda 1 (tekst)
**Odpowiedź /game-state zmienia się na:**
```json
{
  "round_number": 1,
  "round_type": "text",
  "time_left": 20,
  "prompt": null // W pierwszej rundzie nie ma promptu
}
```
Front-end pokazuje:
- Pole tekstowe do wpisania hasła
- Timer odliczający 20 sekund

**Po wpisaniu hasła:**
```javascript
POST /submit {
  "player_id": "uuid",
  "content": {"text": "kot w butach"}
}
```

### 5. Runda 2 (rysowanie)
**Nowa odpowiedź /game-state:**
```json
{
  "round_number": 2,
  "round_type": "drawing",
  "time_left": 40,
  "prompt": "kot w butach" // Losowo przydzielone hasło innego gracza
}
```
Front-end:
1. Wyświetla canvas 800x1000 pikseli
2. Konwertuje rysunek na tablicę RGB (800000 x 3)
3. Po 40s wysyła:

```javascript
POST /submit {
  "player_id": "uuid",
  "content": {
    "drawing": [[255,255,255], [0,0,0], ...] // 800k elementów
  }
}
```

### 6. Kolejne rundy
Wzór powtarza się na przemian:
- Tekst → Rysowanie → Tekst → ...

**Przykładowy prompt w rundzie 3:**
```json
{
  "round_number": 3,
  "round_type": "text",
  "time_left": 20,
  "prompt": {"drawing": [[...]]} // Obrazek z poprzedniej rundy
}
```

### 7. Finał gry
**Odpowiedź /game-state po ostatniej rundzie:**
```json
{
  "status": "finished",
  "chain": [
    {"type": "text", "content": "kot w butach", "author": "Player1"},
    {"type": "drawing", "content": [[...]], "author": "Player3"},
    {"type": "text", "content": "magiczny kocur", "author": "Player2"}
  ]
}
```
Front-end wyświetla historię transformacji hasła w formie "łańcucha skojarzeń"

### 8. Mechanika pollingowa
**Przykładowa implementacja:**
```javascript
let lastUpdate = null;

async function pollGameState() {
  const response = await fetch(`/game-state/${game_id}?player_id=${player_id}`);
  const state = await response.json();
  
  if(state.timestamp !== lastUpdate) {
    lastUpdate = state.timestamp;
    updateUI(state);
  }
  
  setTimeout(pollGameState, 1000);
}
```
