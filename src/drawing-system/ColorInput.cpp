#include "ColorInput.h"

using namespace emscripten;

ColorInput::ColorInput(std::string id) 
    : id(id) {
    inputBox = val::global("document").call<val>("getElementById", id);

    std::string jsCode = 
        "(function() { Module.handleColorCallback(" + std::to_string(reinterpret_cast<uintptr_t>(this)) + "); })";

    val callback = val::global("eval").call<val>("call", val::global("window"), jsCode);

    inputBox.call<void>("addEventListener", std::string("change"), callback);
    inputBox.call<void>("addEventListener", std::string("input"), callback);

    std::cout << "ColorInput initialized with id: " << id << std::endl;
    std::cout << "Input box: " << inputBox["value"].as<std::string>() << std::endl;
}

void ColorInput::onColorChange() {
    std::cout << "Color input changed" << std::endl;
    handleColorChange();
}

void ColorInput::handleColorCallback(uintptr_t ptr) {
    auto* instance = reinterpret_cast<ColorInput*>(ptr);
    instance->onColorChange();
}

Color ColorInput::get_color() {
    return color;
}

void ColorInput::handleColorChange() {
    hex = inputBox["value"].as<std::string>();
    hexToRGB(hex);
    std::cout << "Color changed to: " << hex << std::endl;
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

EMSCRIPTEN_BINDINGS(ColorInput) {
    class_<ColorInput>("ColorInput")
        .constructor<std::string>()
        .function("onColorChange", &ColorInput::onColorChange);

    function("handleColorCallback", &ColorInput::handleColorCallback);
}