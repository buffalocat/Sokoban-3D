#ifndef COLOR_CONSTANTS_H_INCLUDED
#define COLOR_CONSTANTS_H_INCLUDED

struct Color4 {
    float r;
    float g;
    float b;
    float a;
    bool operator==(Color4& c);
};

enum {
    GREEN = 0,
    PINK = 1,
    PURPLE = 2,
    DARK_PURPLE = 3,
    BLUE = 4,
    RED = 5,
    DARK_RED = 6,
    BLACK = 7,
    LIGHT_GREY = 8,
    ORANGE = 9,
    YELLOW = 10,
    GREY = 11,
    DARK_GREY = 12,
    WHITE = 13,
};

const Color4 COLORS[] = {
    Color4{0.6f, 0.9f, 0.7f, 1.0f},
    Color4{0.9f, 0.6f, 0.7f, 1.0f},
    Color4{0.7f, 0.5f, 0.9f, 1.0f},
    Color4{0.3f, 0.2f, 0.6f, 1.0f},
    Color4{0.0f, 0.3f, 0.8f, 1.0f},
    Color4{1.0f, 0.5f, 0.5f, 1.0f},
    Color4{0.6f, 0.0f, 0.1f, 1.0f},
    Color4{0.0f, 0.0f, 0.0f, 1.0f},
    Color4{0.7f, 0.7f, 0.7f, 1.0f},
    Color4{1.0f, 0.7f, 0.3f, 1.0f},
    Color4{0.7f, 0.7f, 0.3f, 1.0f},
    Color4{0.2f, 0.2f, 0.2f, 1.0f},
    Color4{0.1f, 0.1f, 0.1f, 1.0f},
    Color4{1.0f, 1.0f, 1.0f, 1.0f},
};

const int NUM_GREYS = 9;

const Color4 GREYS[NUM_GREYS] = {
    Color4{0.1f, 0.1f, 0.1f, 1.0f},
    Color4{0.2f, 0.2f, 0.2f, 1.0f},
    Color4{0.3f, 0.3f, 0.3f, 1.0f},
    Color4{0.4f, 0.4f, 0.4f, 1.0f},
    Color4{0.5f, 0.5f, 0.5f, 1.0f},
    Color4{0.6f, 0.6f, 0.6f, 1.0f},
    Color4{0.7f, 0.7f, 0.7f, 1.0f},
    Color4{0.8f, 0.8f, 0.8f, 1.0f},
    Color4{0.9f, 0.9f, 0.9f, 1.0f},
};

#endif // COLOR_CONSTANTS_H_INCLUDED
