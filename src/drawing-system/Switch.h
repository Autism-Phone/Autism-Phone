#pragma once
#include <string>
#include <iostream>
#include <vector>
#include <emscripten.h>
#include <emscripten/html5.h>
#include <emscripten/val.h>

class Switch {
private:
    emscripten::val element;
    std::string elementId;

    bool clicked = false;

    std::vector<Switch*> &switchList;
    
    static EM_BOOL handleClick(int eventType, const EmscriptenMouseEvent* e, void* userData);

public:
    Switch(const std::string& selector, std::vector<Switch*> &switchList);
    ~Switch();
};