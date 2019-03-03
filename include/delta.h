#ifndef DELTA_H
#define DELTA_H

#include <deque>

#include "common.h"

class Room;
class RoomMap;
class GameObject;
class PushBlock;
class SnakeBlock;
class Switchable;
class Switch;
class Signaler;
class Player;
class PlayingState;
class Car;
class GateBody;

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
    ~DeltaFrame();
    void revert();
    void push(std::unique_ptr<Delta>);
    bool trivial();

private:
    std::vector<std::unique_ptr<Delta>> deltas_;
};


class UndoStack {
public:
    UndoStack(unsigned int max_depth);
    ~UndoStack();
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
    CreationDelta(GameObject* obj, RoomMap* room_map);
    ~CreationDelta();
    void revert();

private:
    GameObject* obj_;
    RoomMap* map_;
};


class DeletionDelta: public Delta {
public:
    DeletionDelta(GameObject* obj, RoomMap* room_map);
    ~DeletionDelta();
    void revert();

private:
    GameObject* obj_;
    RoomMap* map_;
};


class AbstractCreationDelta: public Delta {
public:
    AbstractCreationDelta(GameObject* obj, RoomMap* room_map);
    ~AbstractCreationDelta();
    void revert();

private:
    GameObject* obj_;
    RoomMap* map_;
};


class PutDelta: public Delta {
public:
    PutDelta(GameObject* obj, RoomMap* room_map);
    ~PutDelta();
    void revert();

private:
    GameObject* obj_;
    RoomMap* map_;
};


class TakeDelta: public Delta {
public:
    TakeDelta(GameObject* obj, RoomMap* room_map);
    ~TakeDelta();
    void revert();

private:
    GameObject* obj_;
    RoomMap* map_;
};


class MotionDelta: public Delta {
public:
    MotionDelta(GameObject* obj, Point3 dpos, RoomMap* room_map);
    ~MotionDelta();
    void revert();

private:
    GameObject* obj_;
    Point3 dpos_;
    RoomMap* map_;
};


class BatchMotionDelta: public Delta {
public:
    BatchMotionDelta(std::vector<GameObject*> objs, Point3 dpos, RoomMap* room_map);
    ~BatchMotionDelta();
    void revert();

private:
    std::vector<GameObject*> objs_;
    Point3 dpos_;
    RoomMap* map_;
};

// Object motion outside of the map
class AbstractMotionDelta: public Delta {
public:
    AbstractMotionDelta(GameObject* obj, Point3 dpos);
    ~AbstractMotionDelta();
    void revert();

private:
    GameObject* obj_;
    Point3 dpos_;
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
    SwitchableDelta(Switchable* obj, bool active, bool waiting);
    ~SwitchableDelta();
    void revert();

private:
    Switchable* obj_;
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
    Signaler* sig_;
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
    ColorChangeDelta(Car* car);
    ~ColorChangeDelta();
    void revert();

private:
    Car* car_;
};

class GatePosDelta: public Delta {
public:
    GatePosDelta(GateBody* gate_body, Point3 dpos);
    ~GatePosDelta();
    void revert();

private:
    GateBody* gate_body_;
    Point3 dpos_;
};

#endif // DELTA_H
