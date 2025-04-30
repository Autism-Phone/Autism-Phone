#include <malloc.h>
#include <alloca.h>
#include <queue>
#include <cmath>

#include "Utils.h"
#include "Canvas.h"
#include "Inputs.h"
#include "Button.h"
#include "Switch.h"
#include "Api.h"

#include <emscripten.h>
#include <emscripten/stack.h>

#include <SDL2/SDL.h>

const int CANVAS_WIDTH = 1000;
const int CANVAS_HEIGHT = 600;

int brush_size = 10;
Shape brush_shape = Shape::SQUARE;

Color brush_color = {0, 255, 0};

Color background_color = {255, 255, 255};

u32 chosen_button = 0;

Canvas* canvas = nullptr;
Input<Color>* color_input = nullptr;
Input<s32>* size_input = nullptr;
Input<Shape>* shape_input = nullptr;
Button *clear_button = nullptr;
Button* submit_button = nullptr;
Api* api = nullptr;

std::vector<Switch*> switchList;

ButtonState l_mouse = UP;
ButtonState r_mouse = UP;
ButtonState m_mouse = UP;

bool submit = false;

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

    api = new Api();
    api->round_init();

    canvas = new Canvas(CANVAS_WIDTH, CANVAS_HEIGHT, background_color, "Drawing System");
    canvas->brush = Brush(brush_color, brush_size, brush_shape);

    color_input = new Input<Color>("color-picker");
    size_input = new Input<s32>("size-picker");
    shape_input = new Input<Shape>("shape-picker");
    clear_button = new Button("clear-button", []() {
        canvas->clear_canvas();
    });

    submit_button = new Button("submit-button", []() {
        Color* pixelBuffer = canvas->get_pixel_buffer();
        api->submit(pixelBuffer);
    });

    switchList.push_back(new Switch("pencil", switchList));
    switchList.push_back(new Switch("eraser", switchList));
}

void input(SDL_Event* event) {
    switch (event->type) {
        case SDL_MOUSEBUTTONDOWN:
            switch (event->button.button) {
                case SDL_BUTTON_LEFT:
                    l_mouse = DOWN;
                    break;
                case SDL_BUTTON_RIGHT:
                    r_mouse = DOWN;
                    break;
                case SDL_BUTTON_MIDDLE:
                    m_mouse = DOWN;
                    break;
                default:
                    break;
            }

            break;
        case SDL_MOUSEBUTTONUP:
            switch (event->button.button) {
                case SDL_BUTTON_LEFT:
                    l_mouse = UP;
                    break;
                case SDL_BUTTON_RIGHT:
                    r_mouse = UP;
                    break;
                case SDL_BUTTON_MIDDLE:
                    m_mouse = UP;
                    break;
                default:
                    break;
            }

            break;
        default:
            break;
    }

    if (l_mouse == DOWN || r_mouse == DOWN) {
        canvas->drawing = true;
        canvas->draw();
    } else {
        canvas->drawing = false;
    }
}

void tool_input () {
    if (switchList[chosen_button]->clicked) {
        return;
    }

    chosen_button = 0;

    for (Switch* sw : switchList) {
        if (sw->clicked) break;
        chosen_button++;
    }

    switch (chosen_button) {
        case 0:
            canvas->erasing = false;
            break;
        case 1:
            canvas->erasing = true;
            break;
        default:
            break;
    }
}

void main_loop() {
    canvas->render_frame();

    tool_input();

    while (SDL_PollEvent(canvas->event)) {
        if (!submit) input(canvas->event);
    }

    if(!canvas->erasing) {
        brush_color = color_input->get_value();
        canvas->brush.color = brush_color;    
    }
    
    brush_size = size_input->get_value();
    canvas->brush.size = brush_size;

    brush_shape = shape_input->get_value();
    canvas->brush.shape = brush_shape;

    canvas->get_mouse_pos();
    if(!submit) canvas->draw();

    if(!submit) canvas->check_update();
}

int main() {
    init();

    emscripten_set_main_loop(main_loop, 0, 1);

    return 0;
}
