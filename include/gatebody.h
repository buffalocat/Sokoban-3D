#ifndef GATEBODY_H
#define GATEBODY_H

#include "pushblock.h"

class Gate;
class GateTransitionAnimation;
class MoveProcessor;

// The part of a Gate that comes up above the ground
// It inherits the color, pushability, and gravitability of its corresponding Gate object
class GateBody: public PushBlock {
public:
    GateBody(Gate* gate, Point3 pos);
    GateBody(Point3 pos, int color, bool pushable, bool gravitable);
    GateBody(const GateBody&);
    ~GateBody();

    std::string name();
    ObjCode obj_code();
    void serialize(MapFileO& file);
    bool skip_serialization();
    static std::unique_ptr<GameObject> deserialize(MapFileI& file);

    Point3 gate_pos();
    void set_gate(Gate*);
    Point3 update_gate_pos(DeltaFrame*);

    void collect_special_links(RoomMap*, Sticky, std::vector<GameObject*>&);

    void set_gate_transition_animation(bool state, MoveProcessor*);
    bool update_state_animation();
    void reset_state_animation();
    bool state_animation();

    void draw(GraphicsManager*);

private:
    Gate* gate_;
    Point3 gate_pos_;

    std::unique_ptr<GateTransitionAnimation> transition_animation_;

    friend class GatePosDelta;
};

#endif // GATEBODY_H
