#pragma once

#include "Utils.h"

#include <emscripten.h>
#include <emscripten/val.h>

class ListInput {
private:
    emscripten::val inputBox;

    int element = 10;

public:
    ListInput(std::string id);
    std::string id;

    int get_element();
};