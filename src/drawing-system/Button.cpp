#include "Button.h"

EM_BOOL Button::handleClick(int eventType, const EmscriptenMouseEvent* e, void* userData) {
    Button* self = static_cast<Button*>(userData);
    if (self && self->userCallback) {
        self->userCallback();
    }
    return EM_TRUE;
}

Button::Button(const std::string& buttonId, std::function<void()> callback) 
    : elementId(buttonId[0] == '#' ? buttonId : "#" + buttonId),
      userCallback(callback) 
{
    emscripten::val element = emscripten::val::global("document").call<emscripten::val>("querySelector", elementId);
    if (element.isNull()) {
        std::cerr << "Button element not found: " << elementId << std::endl;
    }
    
    emscripten_set_click_callback(
        elementId.c_str(),
        this,
        EM_FALSE,
        &Button::handleClick
    );
}

Button::~Button() {
    emscripten_set_click_callback(
        elementId.c_str(),
        nullptr,
        EM_FALSE,
        nullptr
    );
}