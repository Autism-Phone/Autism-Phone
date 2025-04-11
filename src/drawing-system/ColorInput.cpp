#include "ColorInput.h"

using namespace emscripten;

ColorInput::ColorInput(std::string id) : id(id) {
    inputBox = val::global("document").call<val>("getElementById", id);
}

Color ColorInput::get_color() {
    hex = inputBox["value"].as<std::string>();
    hexToRGB(hex);
    return color;
}

void ColorInput::hexToRGB(const std::string &hex)
{
    if (hex.length() == 7 && hex[0] == '#') {
        std::stringstream ss;
        int r, g, b;
        ss << std::hex << hex.substr(1, 2) << " " << hex.substr(3, 2) << " " << hex.substr(5, 2);
        ss >> r >> g >> b;
        color = {(u8)(r), (u8)(g), (u8)(b)};
    }
}