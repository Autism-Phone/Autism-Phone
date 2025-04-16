// Button.h
#pragma once

#include <string>
#include <functional>
#include <emscripten.h>
#include <emscripten/val.h>
#include <emscripten/bind.h>

class Button {
public:
    Button(const std::string &id, std::function<void()> callback);

    void onClick();

private:
    std::string id;
    emscripten::val element;
    std::function<void()> onClickCallback;
};
