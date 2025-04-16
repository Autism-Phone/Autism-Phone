#include "Button.h"
#include <emscripten/val.h>
#include <string>
#include <iostream>

using namespace emscripten;

Button::Button(const std::string &id, std::function<void()> callback) : id(id) {
    onClickCallback = callback;
    element = val::global("document").call<val>("getElementById", id);

    element.call<void>(
        "addEventListener", 
        std::string("click"), 
        val::module_property("onClick")
    );
}

void Button::onClick() {
    if (onClickCallback) onClickCallback();
}

EMSCRIPTEN_BINDINGS(button_bindings) {
    class_<Button>("Button")
        .constructor<std::string, std::function<void()>>()
        .function("onClick", &Button::onClick);
}