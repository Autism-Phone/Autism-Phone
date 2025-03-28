#include "Canvas.h"

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

void Canvas::render_frame() {
    SDL_RenderClear(renderer);

    SDL_UpdateTexture(texture, nullptr, pixelBuffer, sizeof(Color) * width);
    SDL_RenderCopy(renderer, texture, nullptr, nullptr);
    SDL_RenderPresent(renderer);
}

void Canvas::get_mouse_pos() {
    last_mouse_pos = current_mouse_pos;
    SDL_GetMouseState(&current_mouse_pos.x, &current_mouse_pos.y);
}

void Canvas::draw() {
    if (!drawing) {
        return;
    }

    int dx = abs(current_mouse_pos.x - last_mouse_pos.x);
    int dy = abs(current_mouse_pos.y - last_mouse_pos.y);
    int steps = std::max(dx, dy);

    if (steps == 0) {
        update_queue.push(ScreenObject(current_mouse_pos, brush.size, brush.shape, brush.color));
        return;
    }

    float x_inc = (float)(current_mouse_pos.x - last_mouse_pos.x) / steps;
    float y_inc = (float)(current_mouse_pos.y - last_mouse_pos.y) / steps;
    
    float x = last_mouse_pos.x;
    float y = last_mouse_pos.y;

    for (int i = 0; i <= steps; i++) {
        Position interpolated_pos = { (int)(x), (int)(y) };
        update_queue.push(ScreenObject(interpolated_pos, brush.size, brush.shape, brush.color));
        x += x_inc;
        y += y_inc;
    }
}

void Canvas::input() {
    switch (event->type) {
        case SDL_MOUSEBUTTONDOWN:
            drawing = true;
            break;
        case SDL_MOUSEBUTTONUP:
            drawing = false;
            break;
        default:
            break;
    }
}

void Canvas::check_update() {
    double current_time = SDL_GetTicks();
    double delta_time = current_time - last_time;
    last_time = current_time;

    last_update += delta_time;

    if (last_update >= update_interval) {
        update_screen();
        last_update = 0;
    }
}

void Canvas::update_screen() {
    while(!update_queue.empty()) {
        ScreenObject currentObject = update_queue.front();
        currentObject.draw(pixelBuffer, width, height);
        update_queue.pop();
    }
}
