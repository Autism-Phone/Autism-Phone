#include "Utils.h"

#include <emscripten.h>
#include <emscripten/val.h>
#include <emscripten/bind.h>

#include <sstream>

class ColorInput {
private:
    emscripten::val inputBox;

    void handleColorChange();
    void hexToRGB(const std::string &hex);
    
    std::string hex;
    Color color = {0, 0, 0};

public:
    ColorInput(std::string id);
    std::string id;
    void onColorChange();

    static void handleColorCallback(uintptr_t ptr);

    Color get_color();
};