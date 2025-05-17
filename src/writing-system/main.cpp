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

            pixelBuffer = reinterpret_cast<Color*>(decodedData.data());

            canvas = new Canvas(800, 600, Color{255, 255, 255}, "canvas");
            canvas->draw(pixelBuffer);

            free(pixelBuffer);
        }

    } catch (json::parse_error& e) {
        std::cerr << "JSON parse error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}