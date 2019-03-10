#ifndef GATE_H
#define GATE_H

#include "switchable.h"


class GateBody;
class GraphicsManager;
class MapFileO;

class Gate: public Switchable {
public:
    Gate(GameObject* parent, GateBody* body, int color, bool def, bool active, bool waiting);
    virtual ~Gate();

    std::string name();
    ModCode mod_code();
    void serialize(MapFileO& file);
    static void deserialize(MapFileI&, RoomMap*, GameObject*);

    void shift_internal_pos(Point3 d);

    void map_callback(RoomMap*, DeltaFrame*, MoveProcessor*);
    void collect_sticky_links(RoomMap*, Sticky, std::vector<GameObject*>&);

    bool can_set_state(bool state, RoomMap*);
    void apply_state_change(RoomMap*, DeltaFrame*, MoveProcessor*);

    void setup_on_put(RoomMap*);
    void cleanup_on_take(RoomMap*);

    void draw(GraphicsManager*, FPoint3);

    std::unique_ptr<ObjectModifier> duplicate(GameObject*, RoomMap*, DeltaFrame*);

    int color_;

private:
    GateBody* body_;

    friend class ModifierTab;
};

#endif // GATE_H
