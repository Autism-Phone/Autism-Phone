#include "Utils.h"

class Canvas {
private:

public:
    Canvas(int width, int height, Color background_color);

    ~Canvas();

    Color* pb;
    u32 width;
    u32 height;
    Color background_color;
    Allocation alloc_type;
};