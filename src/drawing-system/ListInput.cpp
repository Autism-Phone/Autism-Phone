#include "ListInput.h"

using namespace emscripten;

ListInput::ListInput(std::string id) {
    inputBox = val::global("document").call<val>("getElementById", id);
}

int ListInput::get_element() {
    element = inputBox["value"].as<int>();
    return element;
}
