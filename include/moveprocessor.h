#ifndef MOVEPROCESSOR_H
#define MOVEPROCESSOR_H

#include "common.h"

class Player;
class Block;
class RoomMap;
class DeltaFrame;
class SnakeBlock;
class Component;

class MoveProcessor {
public:
    MoveProcessor(Player*, RoomMap*, Point3 dir);
    ~MoveProcessor();
    void try_move(DeltaFrame*);
    void move_bound(DeltaFrame*);
    void move_general(DeltaFrame*);

    bool move_component(Component*);
    bool try_push(Component*, Point3);

private:
    Player* player_;
    RoomMap* map_;
    Point3 dir_;
    std::vector<std::unique_ptr<Component>> comps_;
};

#endif // MOVEPROCESSOR_H
