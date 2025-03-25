#include <emscripten.h>
#include <emscripten/stack.h>
#include <iostream>
#include <malloc.h>
#include <emscripten/html5.h>

struct Pixel {
    uint8_t r, g, b;
};

Pixel* pixels;

EM_JS(void, setup, (), {
    const div = document.getElementById('drawing-box');
    div.addEventListener('mousemove', (event) => {
        const rect = div.getBoundingClientRect();
        const relativeX = event.clientX - rect.left;
        const relativeY = event.clientY - rect.top;
        Module._on_mouse_move(relativeX, relativeY);
    });
});

extern "C" {
    EMSCRIPTEN_KEEPALIVE
    void on_mouse_move(int x, int y) {
        std::cout << "mouse_pos = (" << x << ", " << y << ")" << std::endl;
    }
}

void main_loop() {
    
}

int main() {
    setup();
    int width, height;
    EM_ASM({
        const div = document.getElementById('drawing-box');
        const rect = div.getBoundingClientRect();
        Module.HEAP32[$0 >> 2] = rect.width;
        Module.HEAP32[$1 >> 2] = rect.height;
    }, &width, &height);

    std::cout << "width = " << width << ", height = " << height << std::endl;

    size_t free_stack = emscripten_stack_get_free();
    std::cout << "free_stack = " << free_stack << std::endl;

    size_t pixels_size = width * height * sizeof(Pixel);
    std::cout << "pixels_size = " << pixels_size << std::endl;

    if (free_stack < pixels_size) {
        std::cout << "Not enough stack space!" << std::endl;
        pixels = (Pixel*)malloc(pixels_size);
    } else {
        pixels = (Pixel*)alloca(pixels_size);
    }

    for (int i = 0; i < width * height; i++) {
        pixels[i].r = 128;
        pixels[i].g = 0;
        pixels[i].b = 0;
    }

    EM_ASM({
        const div = document.getElementById('drawing-box');
        const canvas = document.createElement('canvas');
        canvas.width = $0;
        canvas.height = $1;
        div.appendChild(canvas);
        const ctx = canvas.getContext('2d');
        const imageData = ctx.createImageData($0, $1);
        const data = imageData.data;
        const pixels = Module.HEAPU8.subarray($2, $2 + $0 * $1 * 3);
        for (let i = 0; i < $0 * $1; i++) {
            data[i * 4] = pixels[i * 3];
            data[i * 4 + 1] = pixels[i * 3 + 1];
            data[i * 4 + 2] = pixels[i * 3 + 2];
            data[i * 4 + 3] = 255;
        }
        ctx.putImageData(imageData, 0, 0);
    }, width, height, pixels);

    emscripten_set_main_loop(main_loop, 0, 1);

    return 0;
}
