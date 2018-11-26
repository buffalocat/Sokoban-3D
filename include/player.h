#ifndef PLAYER_H
#define PLAYER_H

#include "block.h"

// The player is a "block" because it's "pushable" just like all blocks
class Player: public Block {
public:
    Player(Point3 pos, RidingState state);
    ~Player();
    ObjCode obj_code();
    void serialize(MapFileO& file);
    static GameObject* deserialize(MapFileI& file);
    void set_riding(RidingState);
    RidingState state();
    void toggle_riding(RoomMap* room_map, DeltaFrame*);
    Block* get_car(RoomMap* room_map, bool strict);

    void draw(GraphicsManager*);

private:
    RidingState state_;
};

#endif // PLAYER_H
