#ifndef WALL_H
#define WALL_H

#include "pushblock.h"

class Wall: public PushBlock {
public:
    Wall();
    ~Wall();
    ObjCode obj_code();
    static GameObject* deserialize(MapFileI& file);

    void draw(GraphicsManager*, Point3 p={0,0,0});
};

#endif // WALL_H
