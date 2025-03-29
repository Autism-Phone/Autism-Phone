#include "Utils.h"

#include <emscripten.h>
#include <emscripten/val.h>

#include <sstream>

class ColorInput {
private:
    emscripten::val inputBox;
    emscripten::val colorInput;

    void handleColorChange(emscripten::val event);
    void hexToRGB(const std::string& hex);
    
    Color color = {0, 0, 0};

public:
    ColorInput(std::string id);
    std::string id;
    void onColorChange(emscripten::val event);

    Color get_color();
};