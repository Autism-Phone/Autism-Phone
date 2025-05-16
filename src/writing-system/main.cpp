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

    nlohmann::json jsonResponse;

    api->get_drawing([&](std::string response) {
        jsonResponse = nlohmann::json::parse(response);
    });

    round_number = jsonResponse["round"]["number"].get<int>();

    if (round_number > 1) {
        pixelBuffer = (Color*)malloc(800 * 600 * sizeof(Color));
        std::string encodedData = jsonResponse["prompt"].get<std::string>();

        std::string decodedData = base64_decode(encodedData);

        pixelBuffer = reinterpret_cast<Color*>(decodedData.data());

        canvas = new Canvas(800, 600, Color{255, 255, 255}, "canvas");
        canvas->draw(pixelBuffer);

        free(pixelBuffer);
    }

    return 0;
}