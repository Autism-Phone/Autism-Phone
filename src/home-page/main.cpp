#include "Api.h"
#include "Button.h"
#include "Utils.h"
#include "Inputs.h"

#include <string>

#include <emscripten.h>
#include <emscripten/html5.h>
#include <emscripten/val.h>

using namespace emscripten;

Api* api = nullptr;
Input<std::string>* inviteCodeInput = nullptr;
Input<std::string>* playerNameInput = nullptr;
Button* createGameButton = nullptr;
Button* joinGameButton = nullptr;

int main() {
    api = new Api();
    inviteCodeInput = new Input<std::string>("code-input");
    playerNameInput = new Input<std::string>("name-input");

    createGameButton = new Button("#create-button", []() {
        std::string playerName = playerNameInput->get_value();
        if (playerName.empty()) {
            EM_ASM({
                alert("Please enter a player name before creating a game.");
            });
            return;
        }
        api->playerName = playerName;
        api->create_game();
    });

    joinGameButton = new Button("#join-button", []() {
        std::string inviteCode = inviteCodeInput->get_value();
        std::string playerName = playerNameInput->get_value();
        if (inviteCode.empty() || playerName.empty()) {
            EM_ASM({
                alert("Please enter both invite code and player name before joining.");
            });
            return;
        }
        api->join_game(inviteCode, playerName);
    });
    
    return 0;
}