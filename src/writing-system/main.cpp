#include "Api.h"
#include "Utils.h"
#include "Button.h"
#include "Inputs.h"

#include <iostream>

#include <emscripten.h>

Api* api = nullptr;
Input<std::string>* inputBox = nullptr;
Button* submitButton = nullptr;

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

    return 0;
}