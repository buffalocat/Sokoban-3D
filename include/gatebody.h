#ifndef GATEBODY_H
#define GATEBODY_H

#include "pushblock.h"

class Gate;

// The part of a Gate that comes up above the ground
// It inherits the color, pushability, and gravitability of its corresponding Gate object
class GateBody: public PushBlock {
public:
    GateBody(Gate* parent);
    GateBody(Point3 pos, int color, bool pushable, bool gravitable);
    ~GateBody();

    std::string name();
    ObjCode obj_code();
    void serialize(MapFileO& file);
    bool skip_serialization();
    static std::unique_ptr<GameObject> deserialize(MapFileI& file);

    void collect_special_links(RoomMap*, Sticky, std::vector<GameObject*>&);

    void draw(GraphicsManager*);

    Gate* parent_;
};

#endif // GATEBODY_H
