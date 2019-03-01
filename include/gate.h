#ifndef GATE_H
#define GATE_H

#include "switchable.h"
#include "common.h"

class GateBody;
class GraphicsManager;
class MapFileO;

class Gate: public Switchable {
public:
    Gate(GameObject* parent, GateBody* body, int color, bool def, bool active, bool waiting);
    virtual ~Gate();

    ModCode mod_code();
    void serialize(MapFileO& file);
    static void deserialize(MapFileI&, RoomMap*, GameObject*);

    void collect_sticky_links(RoomMap*, Sticky, std::vector<GameObject*>&);

    void map_callback(RoomMap*, DeltaFrame*, MoveProcessor*);

    bool can_set_state(bool state, RoomMap*);
    void apply_state_change(RoomMap*, MoveProcessor*);

    void setup_on_put(RoomMap*);
    void cleanup_on_take(RoomMap*);

    void draw(GraphicsManager*, FPoint3);

    std::unique_ptr<ObjectModifier> duplicate(GameObject*);

    int color_;

private:
    GateBody* body_;
};

#endif // GATE_H
