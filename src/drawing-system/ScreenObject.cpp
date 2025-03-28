#include "ScreenObject.h"

ScreenObject::ScreenObject(Position center, int radius, Shape shape, Color color)
    : center(center), radius(radius), shape(shape), color(color) {}

void ScreenObject::SetPixel(int x, int y, Color *PixelBuffer, int CANVAS_WIDTH, int CANVAS_HEIGHT) {
    if (x >= 0 && x < CANVAS_WIDTH && y >= 0 && y < CANVAS_HEIGHT) {
        PixelBuffer[y * CANVAS_WIDTH + x] = color;
    }
}

void ScreenObject::draw_circle(Color *PixelBuffer, int CANVAS_WIDTH, int CANVAS_HEIGHT)
{
    for (int y = -radius; y <= radius; y++) {
        for (int x = -radius; x <= radius; x++) {
            if (x * x + y * y <= radius * radius) {
                SetPixel(center.x + x, center.y + y, PixelBuffer, CANVAS_WIDTH, CANVAS_HEIGHT);
            }
        }
    }
}

void ScreenObject::draw_square(Color *PixelBuffer, int CANVAS_WIDTH, int CANVAS_HEIGHT) {
    for (int i = 0; i < radius; i++) {
        for (int j = 0; j < radius; j++) {
            int x = center.x + i - radius / 2;
            int y = center.y + j - radius / 2;

            SetPixel(x, y, PixelBuffer, CANVAS_WIDTH, CANVAS_HEIGHT);
        }
    }
}

void ScreenObject::draw(Color *PixelBuffer, int CANVAS_WIDTH, int CANVAS_HEIGHT)
{
    switch (shape) {
        case CIRCLE:
            draw_circle(PixelBuffer, CANVAS_WIDTH, CANVAS_HEIGHT);
            break;
        case SQUARE:
            draw_square(PixelBuffer, CANVAS_WIDTH, CANVAS_HEIGHT);
            break;
        default:
            break;
    } 
}
