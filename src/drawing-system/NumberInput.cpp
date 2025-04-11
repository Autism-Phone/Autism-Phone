#include "NumberInput.h"

using namespace emscripten;

void NumberInput::onNumberChange() {
    number = inputBox["value"].as<int>();
    std::cout << "Number changed to: " << number << std::endl;
}

NumberInput::NumberInput(std::string id)
{
    inputBox = val::global("document").call<val>("getElementById", id);

    std::string jsCode =
        "(function() { Module.handleChangeCallback(" + std::to_string(reinterpret_cast<uintptr_t>(this)) + "); })";

    val callback = val::global("eval").call<val>("call", val::global("window"), jsCode);

    inputBox.call<void>("addEventListener", std::string("change"), callback);
}

void NumberInput::handleChangeCallback(uintptr_t ptr) {
    auto* instance = reinterpret_cast<NumberInput*>(ptr);
    instance->onNumberChange();
}

int NumberInput::get_number() {
    return number;
}

EMSCRIPTEN_BINDINGS(NumberInput) {
    class_<NumberInput>("NumberInput")
        .constructor<std::string>()
        .function("onNumberChange", &NumberInput::onNumberChange);

    function("handleChangeCallback", &NumberInput::handleChangeCallback);
}