#include "NumberInput.h"

using namespace emscripten;

NumberInput::NumberInput(std::string id) : id(id) {
    inputBox = val::global("document").call<val>("getElementById", id);
}

int NumberInput::get_number() {
    number = inputBox["value"].as<int>();
    return number;
}