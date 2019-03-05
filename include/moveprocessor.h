#ifndef MOVEPROCESSOR_H
#define MOVEPROCESSOR_H

#include <vector>

class Player;
class GameObject;
class PlayingState;
class RoomMap;
class DeltaFrame;
class SnakeBlock;
class Door;

class Point3;

enum class MoveStep {
    Horizontal = 1,
    Fall = 2,
    ColorChange = 3,
    DoorInt,
    DoorExt,
};

class MoveProcessor {
public:
    MoveProcessor(PlayingState*, RoomMap*, DeltaFrame*);
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

    PlayingState* playing_state_;
    RoomMap* map_;
    DeltaFrame* delta_frame_;

    unsigned int frames_;
    MoveStep state_;
};

#endif // MOVEPROCESSOR_H
