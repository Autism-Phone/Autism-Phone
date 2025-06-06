#pragma once

#include <iostream>

typedef unsigned char u8;
typedef char s8;
typedef unsigned short u16;
typedef short s16;
typedef unsigned int u32;
typedef int s32;
typedef unsigned long long u64;
typedef long long s64;

struct Color {
    u8 r, g, b;
} __attribute__((packed));;

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

enum Allocation {
    STACK,
    HEAP
};

enum ButtonState {
    DOWN,
    UP
};

enum GameState {
    START,
    PLAYING,
    END
};

enum GameType {
    DRAWING,
    PROMPTING
};

enum StateTypes {
    TIME,
    STATUS,
    ROUND_TYPE,
    ROUND_NUMBER,
    SUBMITTED
};