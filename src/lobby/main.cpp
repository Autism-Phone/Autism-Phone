#include "Api.h"
#include "Button.h"
#include "Utils.h"

#include <emscripten.h>
#include <emscripten/html5.h>
#include <emscripten/val.h>

using namespace emscripten;

Api* api = nullptr;

int main() {
    api = new Api();

    Button* createGameButton = new Button("#create-button", []() {
        api->create_game();
    });
    

    return 0;
}