#include "Api.h"
#include "Button.h"
#include "Utils.h"
#include "TextOutput.h"

#include <string>

#include <emscripten.h>
#include <emscripten/html5.h>
#include <emscripten/val.h>

using namespace emscripten;

Api* api = nullptr;
TextOutput* textOutput = nullptr;

int main() {
    api = new Api();
    api->connect_websocket();

    Button* startGameButton = new Button("#start-button", []() {
        api->start_game();
    });

    textOutput = new TextOutput("code");

    std::string inviteCode = val::global("sessionStorage").call<val>("getItem", std::string("inviteCode")).as<std::string>();
    if (!inviteCode.empty()) {
        textOutput->change_text(inviteCode);
    } else {
        std::cerr << "Invite code not found in sessionStorage." << std::endl;
    } 

    return 0;
}