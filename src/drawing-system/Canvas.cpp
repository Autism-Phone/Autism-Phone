#include "Canvas.h"
#include <emscripten/stack.h>

Canvas::Canvas(int width, int height, Color background_color, std::string name) 
    : width(width), height(height), background_color(background_color), name(name) {
    const char* cname = name.c_str();

    window = SDL_CreateWindow(cname, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);

    if (window == nullptr) {
        std::cerr << SDL_GetError() << std::endl;
        SDL_Quit();
        throw("Failed to create window");
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    
    if (renderer == nullptr) {
        std::cerr << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        throw("Failed to create renderer");
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

    size_t free_stack = emscripten_stack_get_free();
    size_t pixels_size = width * height * sizeof(Color);

    if (free_stack < pixels_size) {
        std::cerr << "Not enough stack space" << std::endl;
        pixelBuffer = (Color*)malloc(pixels_size);
        alloc_type = HEAP;
    } else {
        std::cout << "Using stack memory" << std::endl;
        pixelBuffer = (Color*)alloca(pixels_size);
        alloc_type = STACK;
    }

    for (int i = 0; i < width * height; i++) {
        pixelBuffer[i] = background_color;
    }
}

Canvas::~Canvas() {
    if (alloc_type == HEAP) {
        free(pixelBuffer);
    }
    
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    delete event;
}