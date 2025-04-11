#pragma once

#include "Utils.h"

#include <emscripten.h>
#include <emscripten/val.h>

class NumberInput {
private:
    emscripten::val inputBox;

    int number = 10;

public:
    NumberInput(std::string id);
    std::string id;

    int get_number();
};