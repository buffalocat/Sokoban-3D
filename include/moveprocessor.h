#ifndef MOVEPROCESSOR_H
#define MOVEPROCESSOR_H

#include "common.h"

class Player;
class GameObject;
class RoomMap;
class DeltaFrame;
class SnakeBlock;
class Door;

enum class MoveStep {
    Horizontal = 1,
    Fall = 2,
    DoorInt = 3,
    DoorExt = 4,
};

class MoveProcessor {
public:
    MoveProcessor(RoomMap*, DeltaFrame*);
    ~MoveProcessor();

    bool try_move(Player*, Point3);
    void color_change(Player*);

    void perform_switch_checks();
    void try_fall_step();

    void try_door_move(Door*);

    void add_to_fall_check(GameObject*);

    bool update();
    void abort();

private:
    void move_bound(Player*, Point3);
    void move_general(Point3);

    std::vector<GameObject*> moving_blocks_;
    std::vector<GameObject*> fall_check_;

    RoomMap* map_;
    DeltaFrame* delta_frame_;

    unsigned int frames_;
    MoveStep state_;
};

#endif // MOVEPROCESSOR_H
