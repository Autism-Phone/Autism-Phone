#include <malloc.h>
#include <alloca.h>
#include <queue>
#include <cmath>

#include "Utils.h"
#include "Canvas.h"
#include "ColorInput.h"
#include "NumberInput.h"

#include <emscripten.h>
#include <emscripten/stack.h>
#include <emscripten/html5.h>
#include <SDL2/SDL.h>

const int CANVAS_WIDTH = 1000;
const int CANVAS_HEIGHT = 600;

int brush_size = 10;
Shape brush_shape = Shape::SQUARE;

Color brush_color = {0, 255, 0};

Color background_color = {128, 0, 0};

Canvas* canvas = nullptr;
ColorInput* color_input = nullptr;
NumberInput* size_input = nullptr;

void init() {
    SDL_version compiled;
    SDL_version linked;

    SDL_VERSION(&compiled);
    SDL_GetVersion(&linked);

    printf("Compiled against SDL %d.%d.%d\n", 
        compiled.major, compiled.minor, compiled.patch);
    printf("Linked against SDL %d.%d.%d\n", 
        linked.major, linked.minor, linked.patch);

	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		throw("SDL failed to initialise");
	}

    canvas = new Canvas(CANVAS_WIDTH, CANVAS_HEIGHT, background_color, "Drawing System");
    canvas->brush = Brush(brush_color, brush_size, brush_shape);

    color_input = new ColorInput("color-picker");
    size_input = new NumberInput("size-picker");
}

void main_loop() {
    canvas->render_frame();

    while (SDL_PollEvent(canvas->event)) {
        canvas->input();
    }

    brush_color = color_input->get_color();
    canvas->brush.color = brush_color;

    brush_size = size_input->get_number();
    canvas->brush.size = brush_size;
    canvas->brush.shape = brush_shape;

    canvas->get_mouse_pos();
    canvas->draw();

    canvas->check_update();
}

int main() {
    init();

    emscripten_set_main_loop(main_loop, 0, 1);

    return 0;
}
