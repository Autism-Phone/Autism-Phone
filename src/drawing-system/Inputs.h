#pragma once

#include "Utils.h"

#include <emscripten.h>
#include <emscripten/val.h>

#include <sstream>

using namespace emscripten;

template <typename T>
class Input {
private:
    val inputBox;

    T value;

public:
    Input(std::string id);
    std::string id;

    T get_value();
};

template <>
class Input<Color> {
private:
    val inputBox;
    Color value;

    void hexToRGB(const std::string &hex) {
        if (hex.length() == 7 && hex[0] == '#') {
            std::stringstream ss;
            int r, g, b;
            ss << std::hex << hex.substr(1, 2) << " " << hex.substr(3, 2) << " " << hex.substr(5, 2);
            ss >> r >> g >> b;
            value = {(u8)(r), (u8)(g), (u8)(b)};
        }
    }

public:
    Input(std::string id) {
        inputBox = val::global("document").call<val>("getElementById", id);
    }

    std::string id;

    Color get_value() {
        std::string hex = inputBox["value"].as<std::string>();
        hexToRGB(hex);
        return value;
    }
};

template <typename T>
inline Input<T>::Input(std::string id) {
    inputBox = val::global("document").call<val>("getElementById", id);
}

template <typename T>
inline T Input<T>::get_value() {
    value = inputBox["value"].as<T>();
    return value;
}

template <>
inline Shape Input<Shape>::get_value() {
    s32 temp = inputBox["value"].as<s32>();
    value = static_cast<Shape>(temp);
    return value;
}
