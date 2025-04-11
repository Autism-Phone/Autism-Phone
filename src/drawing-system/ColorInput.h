#pragma once

#include "Utils.h"

#include <emscripten.h>
#include <emscripten/val.h>

#include <sstream>

class ColorInput {
private:
    emscripten::val inputBox;

    void hexToRGB(const std::string &hex);
    
    std::string hex;
    Color color = {0, 0, 0};

public:
    ColorInput(std::string id);
    std::string id;

    Color get_color();
};