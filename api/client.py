import requests
import random
import time
from threading import Thread
from queue import Queue

BASE_URL = "http://0.0.0.0:2137"
RESULTS = Queue()

class PlayerClient:
    def __init__(self, name, game_id=None, invite_code=None):
        self.name = name
        self.game_id = game_id
        self.invite_code = invite_code
        self.player_id = None
        self.submission_history = {}
        
    def join_game(self):
        response = requests.post(
            f"{BASE_URL}/join-game",
            json={"invite_code": self.invite_code, "name": self.name}
        )
        if response.status_code == 200:
            data = response.json()
            self.player_id = data['player_id']
            self.game_id = data['game_id']
            print(f"{self.name} joined game {self.game_id}")
        else:
            print(f"{self.name} join error: {response.text}")

    def play_round(self, round_info):
        prompt = round_info.get('prompt')
        round_num = round_info['round']['number']
        round_type = round_info['round']['type']
        
        print(f"\n{self.name} starting round {round_num} ({round_type})")
        print(f"Prompt: {prompt[:50] if prompt else 'First round - no prompt'}")

        # Generuj odpowiedź na podstawie typu rundy
        if round_type == "text":
            content = {"text": f"{self.name}'s answer to: {prompt}"}
        else:
            content = {"drawing": [[random.randint(0,255) for _ in range(3)] for _ in range(10)]}  # Uproszczony rysunek

        # Wyślij odpowiedź
        response = requests.post(
            f"{BASE_URL}/submit",
            json={"player_id": self.player_id, "content": content}
        )
        
        if response.status_code == 200:
            print(f"{self.name} submitted {round_type} successfully")
            self.submission_history[round_num] = content
        else:
            print(f"{self.name} submission error: {response.text}")

    def monitor_game(self):
        last_round = -1
        while True:
            response = requests.get(
                f"{BASE_URL}/game-state/{self.game_id}",
                params={"player_id": self.player_id}
            )
            
            if response.status_code != 200:
                print(f"{self.name} game state error: {response.text}")
                time.sleep(1)
                continue

            game_state = response.json()
            
            if game_state.get('status') == 'finished':
                print(f"\n{self.name} received final state!")
                RESULTS.put(game_state)
                break
                
            if game_state.get('status') == 'in_progress':
                current_round = game_state['round']['number']
                if current_round != last_round:
                    last_round = current_round
                    self.play_round(game_state)
            
            time.sleep(0.5)

def test_full_game():
    # Tworzenie gry
    host = PlayerClient("Host")
    create_response = requests.post(f"{BASE_URL}/create-game")
    game_data = create_response.json()
    host.game_id = game_data['game_id']
    host.invite_code = game_data['invite_code']
    
    print(f"Created game ID: {host.game_id}")
    print(f"Invite code: {host.invite_code}")

    # Dołączanie graczy
    players = [
        PlayerClient("Alice", invite_code=host.invite_code),
        PlayerClient("Bob", invite_code=host.invite_code),
        PlayerClient("Charlie", invite_code=host.invite_code)
    ]
    
    for p in players:
        p.join_game()
        time.sleep(0.2)

    # Start gry
    print("\nStarting game...")
    start_response = requests.post(f"{BASE_URL}/start-game/{host.game_id}")
    print(f"Start response: {start_response.json()}")

    # Uruchom wątki dla każdego gracza
    threads = []
    for p in players:
        t = Thread(target=p.monitor_game)
        t.start()
        threads.append(t)

    # Czekaj na zakończenie gry
    for t in threads:
        t.join()

    # Pokaż wyniki
    print("\nFinal results:")
    while not RESULTS.empty():
        result = RESULTS.get()
        print(f"Player received: {result.get('message')}")
        
        # Dla pełnego testu można odkomentować aby zobaczyć pełną historię
        # print("Full chain:")
        # for entry in result.get('chain', []):
        #     print(f"Round {entry['round_number']} ({entry['type']}): {entry['content'][:50]}")

if __name__ == "__main__":
    test_full_game()