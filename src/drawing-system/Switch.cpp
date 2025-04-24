#include "Switch.h"

using namespace emscripten;

EM_BOOL Switch::handleClick(int eventType, const EmscriptenMouseEvent* e, void* userData) {
    Switch* self = static_cast<Switch*>(userData);
    if (self && !self->clicked) {
        self->clicked = true;
        self->element.call<void>("click");

        std::cout << "Switch checked: " << self->elementId << std::endl;

        for (Switch* sw : self->switchList) {
            if (sw != self) {
                if (sw->clicked) {
                    sw->clicked = false;

                    std::cout << "Switch unchecked: " << sw->elementId << std::endl;
                }
            }
        }
    }
    return EM_TRUE;
}

Switch::Switch(const std::string& buttonId, std::vector<Switch*>& switchList) 
    : elementId(buttonId[0] == '#' ? buttonId : "#" + buttonId),
      switchList(switchList),
      clicked(false)
{
    element = val::global("document").call<val>("querySelector", elementId);

    if (element.isNull() || element.isUndefined()) {
        std::cerr << "Switch element not found: " << elementId << std::endl;
        return;
    }

    clicked = element.call<bool>("matches", std::string(":checked"));
    
    emscripten_set_click_callback(
        elementId.c_str(),
        this,
        EM_TRUE,
        &Switch::handleClick
    );
}

Switch::~Switch() {
    emscripten_set_click_callback(
        elementId.c_str(),
        nullptr,
        EM_TRUE,
        nullptr
    );
}