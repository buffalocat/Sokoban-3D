#ifndef DELTA_H
#define DELTA_H

#include "common.h"

class Room;
class RoomMap;
class GameObject;
class Block;
class PushBlock;
class SnakeBlock;
class Switchable;
class Switch;
class Signaler;
class Player;
class PlayingState;

struct Point;
enum class Layer;


class Delta {
public:
    virtual ~Delta();
    virtual void revert() = 0;
};


class DeltaFrame {
public:
    DeltaFrame();
    virtual ~DeltaFrame();
    void revert();
    void push(std::unique_ptr<Delta>);
    bool trivial();

private:
    std::vector<std::unique_ptr<Delta>> deltas_;
};


class UndoStack {
public:
    UndoStack(unsigned int max_depth);
    virtual ~UndoStack();
    void push(std::unique_ptr<DeltaFrame>);
    bool non_empty();
    void pop();
    void reset();

private:
    std::deque<std::unique_ptr<DeltaFrame>> frames_;
    unsigned int max_depth_;
    unsigned int size_;
};


class CreationDelta: public Delta {
public:
    CreationDelta(Point3 pos, RoomMap* room_map);
    ~CreationDelta();
    void revert();

private:
    Point3 pos_;
    RoomMap* room_map_;
};


class DeletionDelta: public Delta {
public:
    DeletionDelta(std::unique_ptr<GameObject> object, RoomMap* room_map);
    ~DeletionDelta();
    void revert();

private:
    std::unique_ptr<GameObject> object_;
    RoomMap* room_map_;
};


class MotionDelta: public Delta {
public:
    MotionDelta(std::vector<std::pair<GameObject*, Point3>> pairs, RoomMap* room_map);
    ~MotionDelta();
    void revert();

private:
    std::vector<std::pair<GameObject*, Point3>> pairs_;
    RoomMap* room_map_;
};

/*
class SingleMoveDelta: public Delta {
public:
    SingleMoveDelta(Block* obj, Point3 p, RoomMap* room_map);
    ~SingleMoveDelta();
    void revert();

private:
    Block* obj_;
    Point3 p_;
    RoomMap* room_map_;
};
*/

class FallDelta: public Delta {
public:
    FallDelta(std::vector<Block*> blocks, int distance, RoomMap* room_map);
    ~FallDelta();
    void revert();

private:
    std::vector<Block*> blocks_;
    RoomMap* room_map_;
    int distance_;
};


class AddLinkDelta: public Delta {
public:
    AddLinkDelta(SnakeBlock* a, SnakeBlock* b);
    ~AddLinkDelta();
    void revert();

private:
    SnakeBlock* a_;
    SnakeBlock* b_;
};


class RemoveLinkDelta: public Delta {
public:
    RemoveLinkDelta(SnakeBlock* a, SnakeBlock* b);
    ~RemoveLinkDelta();
    void revert();

private:
    SnakeBlock* a_;
    SnakeBlock* b_;
};


class DoorMoveDelta: public Delta {
public:
    DoorMoveDelta(PlayingState* state, Room* room, Point3 pos);
    ~DoorMoveDelta();
    void revert();

private:
    PlayingState* state_;
    Room* room_;
    Point3 pos_;
};


class SwitchableDelta: public Delta {
public:
    SwitchableDelta(Switchable* obj, bool active, bool waiting, RoomMap* room_map);
    ~SwitchableDelta();
    void revert();

private:
    Switchable* obj_;
    RoomMap* room_map_;
    bool active_;
    bool waiting_;
};


class SwitchToggleDelta: public Delta {
public:
    SwitchToggleDelta(Switch* obj);
    ~SwitchToggleDelta();
    void revert();

private:
    Switch* obj_;
};

class SignalerToggleDelta: public Delta {
public:
    SignalerToggleDelta(Signaler*);
    ~SignalerToggleDelta();
    void revert();

private:
    Signaler* obj_;
};


class RidingStateDelta: public Delta {
public:
    RidingStateDelta(Player* player, RidingState state);
    ~RidingStateDelta();
    void revert();

private:
    Player* player_;
    RidingState state_;
};


class ColorChangeDelta: public Delta {
public:
    ColorChangeDelta(Block* obj);
    ~ColorChangeDelta();
    void revert();

private:
    Block* obj_;
};

#endif // DELTA_H
