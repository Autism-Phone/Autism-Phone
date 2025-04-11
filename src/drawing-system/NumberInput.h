#pragma once

#include "Utils.h"

#include <emscripten.h>
#include <emscripten/val.h>
#include <emscripten/bind.h>


class NumberInput {
private:
    emscripten::val inputBox;

    int number = 10;

public:
    NumberInput(std::string id);
    std::string id;

    static void handleChangeCallback(uintptr_t ptr);
    void onNumberChange();

    int get_number();
};