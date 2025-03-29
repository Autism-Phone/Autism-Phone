#include "ColorInput.h"

ColorInput::ColorInput(std::string id) 
    : id(id) {
    inputBox = emscripten::val::global("document").call<emscripten::val>("getElementById", id);
    
    
}

void ColorInput::onColorChange(emscripten::val event) {
    handleColorChange(event);
}

Color ColorInput::get_color() {
    return color;
}

void ColorInput::hexToRGB(const std::string &hex) {
    if (hex.length() == 7 && hex[0] == '#') {
        std::stringstream ss;
        int r, g, b;
        ss << std::hex << hex.substr(1, 2) << " " << hex.substr(3, 2) << " " << hex.substr(5, 2);
        ss >> r >> g >> b;
        color = {(u8)(r), (u8)(g), (u8)(b)};
    }
}

