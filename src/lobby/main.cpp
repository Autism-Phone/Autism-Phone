#include "Api.h"
#include "Button.h"
#include "Utils.h"

#include <emscripten/html5.h>
#include <emscripten/val.h>

using namespace emscripten;

Api* api = nullptr;

int main() {
    api = new Api();
    
    api->create_game();
    val inviteCodeVal = val::global("localStorage").call<val>("getItem", std::string("inviteCode"));
    std::string inviteCode = inviteCodeVal.isNull() || inviteCodeVal.isUndefined() 
        ? "" 
        : inviteCodeVal.as<std::string>();
    std::string playerName = "joe";
    api->join_game(inviteCode, playerName);
    
    val::global("window").set("location", std::string("drawing-page.html"));

    return 0;
}