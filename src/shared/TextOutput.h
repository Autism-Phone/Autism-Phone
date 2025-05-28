#include "Utils.h"

#include <emscripten.h>
#include <emscripten/val.h>

class TextOutput {
private:
    emscripten::val textAreaContainer;
    std::string id;
    std::string text;

public:
    TextOutput(std::string id);
    void change_text(const std::string &newText);
};