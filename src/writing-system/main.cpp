#include "Api.h"
#include "Utils.h"
#include "Button.h"
#include "Inputs.h"
#include "Canvas.h"
#include "json.hpp"
#include "base64.h"
#include "Compile.h"

#include <iostream>

#include <emscripten.h>

Api* api = nullptr;
Input<std::string>* inputBox = nullptr;
Button* submitButton = nullptr;
Canvas* canvas = nullptr;

Color* pixelBuffer = nullptr;

using json = nlohmann::json;

int main () {
    api = new Api();

    api->round_init();
    inputBox = new Input<std::string>("prompt-input");
    submitButton = new Button("#submit-button", []() {
        std::string text = inputBox->get_value();

        std::cout << "Text: " << text << std::endl;

        if (text.empty()) {
            EM_ASM({
                alert("Please enter a prompt before submitting.");
            });
            return;
        }
        api->submit(text);
    });

    int round_number = 0;

    std::string stringResponse;

    api->get_drawing([&](std::string response) {
        stringResponse = response;
    });

    try {
        json jsonResponse = json::parse(stringResponse);
        std::cout << "Parsed JSON: " << jsonResponse.dump(4) << std::endl;

        std::string prompt = jsonResponse["prompt"].get<std::string>();

        round_number = jsonResponse["round"]["number"].get<int>();

        if (round_number > 1) {
            pixelBuffer = (Color*)malloc(800 * 600 * sizeof(Color));
            std::string encodedData = jsonResponse["prompt"].get<std::string>();

            std::string decodedData = base64_decode(encodedData);
            std::cout << "Decoded data size: " << decodedData.size() << std::endl;
            std::cout << "Decoded data: " << decodedData << std::endl;

            for (int i = 0; i < decodedData.size(); i++) {
                pixelBuffer = reinterpret_cast<Color*>(decodedData[i]);
            }

            SDL_version compiled;
            SDL_version linked;

            SDL_VERSION(&compiled);
            SDL_GetVersion(&linked);

            printf("Compiled against SDL %d.%d.%d\n", 
                compiled.major, compiled.minor, compiled.patch);
            printf("Linked against SDL %d.%d.%d\n", 
                linked.major, linked.minor, linked.patch);

            if (SDL_Init(SDL_INIT_VIDEO) != 0) {
                throw("SDL failed to initialise");
	        }

            canvas = new Canvas(1000, 600, Color{255, 255, 255}, "canvas");
            canvas->draw(pixelBuffer);

            free(pixelBuffer);
        }

    } catch (json::parse_error& e) {
        std::cerr << "JSON parse error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}