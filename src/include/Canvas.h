#include "Utils.h"

class Canvas {
private:

public:
    Canvas(int width, int height, Color background_color);

    ~Canvas();

    Color* pixels;
    int width;
    int height;
    Color background_color;
    bool alloc_type;
};