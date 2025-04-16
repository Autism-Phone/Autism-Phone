#include "Utils.h"
#include "ScreenObject.h"
#include "Brush.h"

#include <string>
#include <queue>

#include <emscripten/stack.h>
#include <SDL2/SDL.h>

class Canvas {
private:
    Allocation alloc_type;
    Color* pixelBuffer;

    Position current_mouse_pos;
    Position last_mouse_pos;

    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    SDL_Texture* texture = nullptr;

    bool drawing = false;

    double last_time = 0;

    const double update_interval = 10.0;
    double last_update = 0.0;

    void update_screen();

public:
    Canvas(int width, int height, Color background_color, std::string name);
    ~Canvas();

    void render_frame();
    void get_mouse_pos();
    void draw();
    void input();
    void check_update();
    void clear_canvas();

    SDL_Event* event = nullptr;

    u32 width;
    u32 height;
    Color background_color;

    std::string name;
    std::queue<ScreenObject> update_queue;
    Brush brush = Brush(Color{0, 255, 0}, 10, Shape::CIRCLE);
};