import requests
from time import sleep


BASE_URL = "http://0.0.0.0:2137"

def create_game():
    url = f"{BASE_URL}/create-game"
    try:
        response = requests.post(url)
        response.raise_for_status()
        return response.json()
    except requests.exceptions.RequestException as e:
        print(f"Error creating game: {e}")
        return None

def join_game(invite_code, player_name):
    url = f"{BASE_URL}/join-game"
    payload = {
        "invite_code": invite_code,
        "name": player_name
    }
    try:
        response = requests.post(url, json=payload)
        response.raise_for_status()
        return response.json()
    except requests.exceptions.RequestException as e:
        print(f"Error joining game: {e}")
        return None
    

def start_game(game_id: str):
    url = f"{BASE_URL}/start-game/{game_id}"
    try:
        response = requests.post(url)
        response.raise_for_status()
        return response.json()
    except requests.exceptions.HTTPError as he:
        print(f"HTTP Error starting game: {he}\nResponse: {response.text}")
    except requests.exceptions.RequestException as e:
        print(f"Request Error starting game: {e}")
    return None


def get_game_state(game_id, player_id):
    url = f"{BASE_URL}/game-state/{game_id}"
    params = {"player_id": player_id}
    try:
        response = requests.get(url, params=params)
        response.raise_for_status()
        return response.json()
    except requests.exceptions.RequestException as e:
        print(f"Error downloading game save: {e}")
        return None


if __name__ == "__main__":
    game_info = create_game()
    if game_info:
        print("Game created:", game_info)
        print()
        game_id = game_info.get("game_id")
        invite_code = game_info.get("invite_code")
        
        player_info = join_game(invite_code, "Player1")
        if player_info:
            player_id = player_info.get("player_id")
            print("Joined game:", player_info)
            print()
            
            game_state = get_game_state(game_id, player_id)
            if game_state:
                print("Game state:", game_state)


        player_info = join_game(invite_code, "Player2")
        if player_info:
            player_id = player_info.get("player_id")
            print("Joined game:", player_info)
            print()
            
            sleep(1)
            game_state = get_game_state(game_id, player_id)
            if game_state:
                print("Game state:", game_state)
            print()


        start_response = start_game(game_id)
        if start_response:
            print("Game started:", start_response)
        print()

        while True:
            sleep(1)
            game_state = get_game_state(game_id, player_id)
            if game_state:
                print("Game state:", game_state)
            print()

