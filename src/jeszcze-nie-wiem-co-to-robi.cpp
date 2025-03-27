#include <emscripten.h>
#include <emscripten/stack.h>
#include <iostream>
#include <malloc.h>

#include <alloca.h> // Tę linijkę nalezy zakomentować na Windowsie

#include <emscripten/html5.h>
#include <SDL/SDL.h>
#include <SDL/SDL_hints.h>

struct Pixel {
    uint8_t r, g, b;
};

struct Position {
    int x, y;
};

Pixel* pixels;

SDL_Window* window = nullptr;
SDL_Renderer* renderer = nullptr;
SDL_Event* event = nullptr;

double last_time = SDL_GetTicks();
const double update_interval = 10.0;
double last_update = 0.0;

Position mouse_pos = {0, 0};

Position get_div_corner() {
    Position pos;
    EM_ASM({
        const div = document.getElementById('drawing-box');
        const rect = div.getBoundingClientRect();
        Module.HEAP32[$0 >> 2] = rect.left;
        Module.HEAP32[$1 >> 2] = rect.top;
    }, &pos.x, &pos.y);
    return pos;
}

std::pair<int, int> get_div_size() {
    int width, height;
    EM_ASM({
        const div = document.getElementById('drawing-box');
        Module.HEAP32[$0 >> 2] = div.clientWidth;
        Module.HEAP32[$1 >> 2] = div.clientHeight;
    }, &width, &height);
    return {width, height};
}

void init() {
	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		throw("SDL failed to initialise");
	}

    auto [width, height] = get_div_size();

	window = SDL_CreateWindow("SDL2 Example!!", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);

	if (window == nullptr) {
		SDL_Quit();
		throw("Failed to create window");
	}

	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	
	if (renderer == nullptr)
	{
		window = NULL;
		SDL_Quit();
		throw("Failed to create renderer");
	}

	event = new SDL_Event();
}

void update_mouse_pos() {
    SDL_GetMouseState(&mouse_pos.x, &mouse_pos.y);
}

void render() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    SDL_RenderPresent(renderer);
}

void input() {
    if (event->type == SDL_QUIT) {
        emscripten_cancel_main_loop();
    }
}

void main_loop() {
    double current_time = SDL_GetTicks();
    double delta_time = current_time - last_time;
    last_time = current_time;

    render();

    while (SDL_PollEvent(event)) {
        input();
    }

    update_mouse_pos();
    std::cout << "(" << mouse_pos.x << ", " << mouse_pos.y << ")" << std::endl;
}

int main() {
    init();

    auto [width, height] = get_div_size();

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

    emscripten_set_main_loop(main_loop, 0, 1);

    return 0;
}
