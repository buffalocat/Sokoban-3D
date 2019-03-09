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
class GateBody;

class Point3;

enum class MoveStep {
    Horizontal = 1,
    PreFallSwitch = 2,
    ColorChange = 3,
    DoorInt,
    DoorExt,
};

class MoveProcessor {
public:
    MoveProcessor(PlayingState*, RoomMap*, DeltaFrame*, bool);
    ~MoveProcessor();

    bool try_move(Player*, Point3);
    void color_change(Player*);

    void try_fall_step();
    void perform_switch_checks(bool skippable);

    void try_door_move(Door*);

    void add_to_fall_check(GameObject*);
    void add_to_moving_blocks(GameObject*);

    void add_gate_transition(GateBody*, bool);
    void update_gate_transitions();

    bool update();
    void abort();

private:
    void move_bound(Player*, Point3);
    void move_general(Point3);

    std::vector<GameObject*> moving_blocks_;
    std::vector<GameObject*> fall_check_;

    std::vector<std::pair<GateBody*, bool>> gate_transitions_;

    PlayingState* playing_state_;
    RoomMap* map_;
    DeltaFrame* delta_frame_;

    unsigned int frames_;
    MoveStep state_;

    bool animated_;
};

#endif // MOVEPROCESSOR_H
