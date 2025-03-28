#include "Utils.h"

class ScreenObject {
private:
    void SetPixel(int x, int y, Color *PixelBuffer, int CANVAS_WIDTH, int CANVAS_HEIGHT);

    void draw_circle(Color *PixelBuffer, int CANVAS_WIDTH, int CANVAS_HEIGHT);

    void draw_square(Color *PixelBuffer, int CANVAS_WIDTH, int CANVAS_HEIGHT);

public:
    ScreenObject(Position center, int radius, Shape shape, Color color);

    Position center;
    int radius;
    Shape shape;
    Color color;

    void draw(Color *PixelBuffer, int CANVAS_WIDTH, int CANVAS_HEIGHT);
};