#include <iostream>
#include <malloc.h>
#include <alloca.h>
#include <queue>
#include <cmath>

#include <emscripten.h>
#include <emscripten/stack.h>
#include <emscripten/html5.h>
#include <SDL2/SDL.h>

struct Color {
    uint8_t r, g, b;
};

struct Position {
    int x, y;
};

enum Tool {
    BRUSH,
    ERASER
};

enum Shape {
    CIRCLE,
    SQUARE
};

Color* pixels;

SDL_Window* window = nullptr;
SDL_Renderer* renderer = nullptr;
SDL_Event* event = nullptr;
SDL_Texture* texture = nullptr;

double last_time = SDL_GetTicks();
const double update_interval = 10.0;
double last_update = 0.0;
bool drawing = false;

const int CANVAS_WIDTH = 1000;
const int CANVAS_HEIGHT = 600;

Position mouse_pos = {0, 0};
Position last_mouse_pos = {0, 0};

Color brush_color = {0, 255, 0};
int brush_size = 10;
Shape brush_shape = CIRCLE;

Color background_color = {128, 0, 0};

class Screen_Object {
private:
    void SetPixel(int x, int y) {
        if (x >= 0 && x < CANVAS_WIDTH && y >= 0 && y < CANVAS_HEIGHT) {
            pixels[y * CANVAS_WIDTH + x] = color;
        }
    }

    void draw_circle() {
        for (int y = -radius; y <= radius; y++) {
            for (int x = -radius; x <= radius; x++) {
                if (x * x + y * y <= radius * radius) {
                    SetPixel(center.x + x, center.y + y);
                }
            }
        }
    }

    void draw_square() {
        for (int i = 0; i < radius; i++) {
            for (int j = 0; j < radius; j++) {
                int x = center.x + i - radius / 2;
                int y = center.y + j - radius / 2;

                SetPixel(x, y);
            }
        }
    }

public:
    Screen_Object(Position center, int radius, Shape shape, Color color) 
        : center(center), radius(radius), shape(shape), color(color) {}

    Position center;
    int radius;
    Shape shape;
    Color color;

    void draw() {
        switch (shape) {
            case CIRCLE:
                draw_circle();
                break;
            case SQUARE:
                draw_square();
                break;
            default:
                break;
        } 
    }
};

std::queue<Screen_Object> update_queue;

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

	window = SDL_CreateWindow("Rysujcie", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, CANVAS_WIDTH, CANVAS_HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);

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

    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STREAMING, CANVAS_WIDTH, CANVAS_HEIGHT);

    if (texture == nullptr) {
        std::cerr << SDL_GetError() << std::endl;
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        throw("Failed to create texture");
    }

	event = new SDL_Event();

    size_t free_stack = emscripten_stack_get_free();
    size_t pixels_size = CANVAS_WIDTH * CANVAS_HEIGHT * sizeof(Color);

    if (free_stack < pixels_size) {
        std::cerr << "Not enough stack space" << std::endl;
        pixels = (Color*)malloc(pixels_size);
    } else {
        std::cout << "Using stack memory" << std::endl;
        pixels = (Color*)alloca(pixels_size);
    }

    for (int i = 0; i < CANVAS_WIDTH * CANVAS_HEIGHT; i++) {
        pixels[i] = background_color;
    }
}

void update_mouse_pos() {
    last_mouse_pos = mouse_pos;
    SDL_GetMouseState(&mouse_pos.x, &mouse_pos.y);
}

void render() {
    SDL_RenderClear(renderer);

    SDL_UpdateTexture(texture, nullptr, pixels, sizeof(Color) * CANVAS_WIDTH);
    SDL_RenderCopy(renderer, texture, nullptr, nullptr);
    SDL_RenderPresent(renderer);
}

void draw() {
    int dx = abs(mouse_pos.x - last_mouse_pos.x);
    int dy = abs(mouse_pos.y - last_mouse_pos.y);
    int steps = std::max(dx, dy);

    if (steps == 0) {
        update_queue.push(Screen_Object(mouse_pos, brush_size, brush_shape, brush_color));
        return;
    }

    float x_inc = (float)(mouse_pos.x - last_mouse_pos.x) / steps;
    float y_inc = (float)(mouse_pos.y - last_mouse_pos.y) / steps;
    
    float x = last_mouse_pos.x;
    float y = last_mouse_pos.y;

    for (int i = 0; i <= steps; i++) {
        Position interpolated_pos = { (int)(x), (int)(y) };
        update_queue.push(Screen_Object(interpolated_pos, brush_size, brush_shape, brush_color));
        x += x_inc;
        y += y_inc;
    }
}


void update_screen() {
    while (!update_queue.empty()) {
        Screen_Object obj = update_queue.front();
        obj.draw();
        update_queue.pop();
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

    update_mouse_pos();

    if (drawing) {
        draw();
    }

    last_update += delta_time;

    if (last_update > update_interval) {
        update_screen();
        last_update -= update_interval;
    }
}

int main() {
    init();

    emscripten_set_main_loop(main_loop, 0, 1);

    return 0;
}
