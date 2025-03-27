#include <emscripten.h>
#include <emscripten/stack.h>
#include <iostream>
#include <malloc.h>

#include <alloca.h> // Tę linijkę nalezy zakomentować na Windowsie

#include <emscripten/html5.h>
#include <SDL2/SDL.h>

struct Pixel {
    uint8_t r, g, b;
};

struct Color {
    uint8_t r, g, b;
};

struct Position {
    int x, y;
};

Pixel* pixels;

SDL_Window* window = nullptr;
SDL_Renderer* renderer = nullptr;
SDL_Event* event = nullptr;
SDL_Texture* texture = nullptr;

double last_time = SDL_GetTicks();
const double update_interval = 10.0;
double last_update = 0.0;
bool drawing = false;

Position mouse_pos = {0, 0};

Color brush_color = {0, 255, 0};
int brush_size = 10;

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

    auto [width, height] = get_div_size();

	window = SDL_CreateWindow("SDL2 Example!!", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);

	if (window == nullptr) {
		SDL_Quit();
		throw("Failed to create window");
	}

	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	
	if (renderer == nullptr) {
        throw("Failed to create renderer");
		SDL_DestroyWindow(window);
		SDL_Quit();
	}

    SDL_RendererInfo info;
    SDL_GetRendererInfo(renderer, &info);
    printf("Renderer: %s, Flags: %d\n", info.name, info.flags);

    for (auto i : info.texture_formats) {
        printf("Texture format: %d\n", i);
    }

    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STREAMING, width, height);

    if (texture == nullptr) {
        std::cerr << SDL_GetError() << std::endl;
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        throw("Failed to create texture");
    }

	event = new SDL_Event();
}

void update_mouse_pos() {
    SDL_GetMouseState(&mouse_pos.x, &mouse_pos.y);
}

void render() {
    auto [width, height] = get_div_size();
    if (width <= 0 || height <= 0) {    
        return;
    }

    SDL_RenderClear(renderer);

    SDL_UpdateTexture(texture, nullptr, pixels, sizeof(Pixel) * width);
    SDL_RenderCopy(renderer, texture, nullptr, nullptr);
    SDL_RenderPresent(renderer);
}

void draw() {
    auto [width, height] = get_div_size();
    if (width <= 0 || height <= 0) {    
        return;
    }

    for (int i = 0; i < brush_size; i++) {
        for (int j = 0; j < brush_size; j++) {
            int x = mouse_pos.x + i - brush_size / 2;
            int y = mouse_pos.y + j - brush_size / 2;

            if (x >= 0 && x < width && y >= 0 && y < height) {
                pixels[y * width + x] = {brush_color.r, brush_color.g, brush_color.b};
            }
        }
    }
}

void input() {
    switch (event->type) {
        case SDL_MOUSEBUTTONDOWN:
            drawing = true;
            break;
        case SDL_MOUSEBUTTONUP:
            drawing = false;
            break;
        case SDL_QUIT:
            emscripten_cancel_main_loop();
            break;
        default:
            break;
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

    if (drawing) {
        draw();
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


    pixels = (Pixel*)malloc(pixels_size);


    for (int i = 0; i < width * height; i++) {
        pixels[i].r = 128;
        pixels[i].g = 0;
        pixels[i].b = 0;
    }

    emscripten_set_main_loop(main_loop, 0, 1);

    return 0;
}
