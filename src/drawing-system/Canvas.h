#include "Utils.h"
#include <SDL2/SDL.h>
#include <string>

class Canvas {
private:
    Allocation alloc_type;
    Color* pixelBuffer;

public:
    Canvas(int width, int height, Color background_color, std::string name);
    ~Canvas();

    void render_frame();
    void get_mouse_pos(Position& pos);

    u32 width;
    u32 height;
    Color background_color;

    std::string name;

    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    SDL_Event* event = nullptr;
    SDL_Texture* texture = nullptr;
};