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
    

def submit(player_id: str, content: dict):
    url = f"{BASE_URL}/submit"
    payload = {
        "player_id": player_id,
        "content": content
    }
    
    try:
        response = requests.post(url, json=payload)
        print("Full server response:", response.text)
        response.raise_for_status()
        return response.json()
    except requests.exceptions.HTTPError as he:
        print(f"HTTP Error Details: {he.response.text}")
    except requests.exceptions.RequestException as e:
        print(f"Connection Error: {e}")
    return None
    

def generate_drawing() -> list:
    width = 2
    height = 5
    
    row = []
    for x in range(width):
        if x % 2 == 0:
            row.append([255, 255, 255])
        else:
            row.append([0, 0, 0])
    
    return row * height


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

        player_info = join_game(invite_code, "Player3")
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

        help_var = 1

        while True:
            sleep(0.5)
            game_state = get_game_state(game_id, player_id)
            
            if not game_state:
                continue
                
            print("Game state:", game_state)
            
            try:
                current_round = game_state.get("round", {})
                time_left = float(current_round.get("time_left", 0.0))
                round_type = current_round.get("type", "")

                round_number = current_round.get("number", {})
                print(round_number)
                
                if all([
                    game_state.get("status") == "in_progress",
                    help_var == round_number,
                    time_left < 0.5,
                    not game_state.get("player_has_submitted", False)
                ]):
                    content = {}
                    
                    if round_type == "text":
                        content["text"] = "test"
                    elif round_type == "drawing":
                        content["drawing"] = generate_drawing()
                    else:
                        print("Nieznany typ rundy")
                        continue
                        
                    result = submit(player_id, content)
                    if result and result.get("status") == "success":
                        print("Odpowiedź wysłana pomyślnie!")
                        help_var += 1
                    
            except Exception as e:
                print(f"Błąd: {e}")