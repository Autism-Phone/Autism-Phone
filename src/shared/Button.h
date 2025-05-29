#pragma once
#include <string>
#include <iostream>
#include <functional>
#include <emscripten.h>
#include <emscripten/html5.h>
#include <emscripten/val.h>

class Button {
private:
    emscripten::val element;
    std::string elementId;
    std::function<void()> userCallback;
    
    static EM_BOOL handleClick(int eventType, const EmscriptenMouseEvent* e, void* userData);

public:
    Button(const std::string& selector, std::function<void()> callback);
    ~Button();
};