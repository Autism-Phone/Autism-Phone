#include <emscripten.h>
#include <iostream>

int main () {
    std::cout << "Hello, World (cpp)!" << std::endl;
    EM_ASM({
        console.log("Hello, World (JS)!");
    });
    return 0;
}