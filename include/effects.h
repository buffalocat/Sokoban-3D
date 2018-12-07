#ifndef EFFECTS_H
#define EFFECTS_H

#include "common.h"

class GraphicsManager;

struct FallTrail {
    Point3 base;
    int height;
    unsigned char opacity;
    unsigned char color;
};

class Effects {
public:
    Effects();
    ~Effects();
    void sort_by_distance(float angle);
    void draw(GraphicsManager*);
    void push_trail(Block*, int height, int drop);

private:
    std::vector<FallTrail> trails_;
};

#endif // EFFECTS_H
