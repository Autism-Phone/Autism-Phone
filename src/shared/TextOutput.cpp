#include "TextOutput.h"

TextOutput::TextOutput(std::string id) : id(id) {
    textAreaContainer = emscripten::val::global("document").call<emscripten::val>("getElementById", id);
    
    if (textAreaContainer.isNull() || textAreaContainer.isUndefined()) {
        std::cerr << "Text area container not found!" << std::endl;
    }
}

void TextOutput::change_text(const std::string &newText) {
    text = newText;
    textAreaContainer.set("value", text);
}
