#ifndef PRESSSWITCH_H
#define PRESSSWITCH_H

#include "common.h"
#include "objectmodifier.h"
#include "switch.h"

class MapFileI;
class MapFileO;
class RoomMap;
class DeltaFrame;
class GraphicsManager;

class PressSwitch: public Switch {
public:
    PressSwitch(GameObject* parent, unsigned char color, bool persistent, bool active);
    ~PressSwitch();

    ModCode mod_code();
    void serialize(MapFileO& file);
    static ObjectModifier* deserialize(GameObject*, MapFileI& file);

    void check_send_signal(RoomMap*, DeltaFrame*);
    bool should_toggle(RoomMap*);

    void setup_on_put(RoomMap*);
    void cleanup_on_take(RoomMap*);

    void draw(GraphicsManager*);

    void map_callback(RoomMap*, DeltaFrame*);

    void check_above_occupied(RoomMap*, DeltaFrame*);
    void check_above_vacant(RoomMap*, DeltaFrame*);

private:
    unsigned char color_;
};

#endif // PRESSSWITCH_H
