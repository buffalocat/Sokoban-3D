#ifndef DELTA_H
#define DELTA_H

#include "common.h"

class RoomManager;
class Room;
class RoomMap;
class GameObject;
class Block;
class PushBlock;
class SnakeBlock;

struct Point;

enum class Layer;

class Delta {
public:
    virtual ~Delta() {};
    virtual void revert() = 0;
};

class DeltaFrame {
public:
    DeltaFrame();
    void revert();
    void push(std::unique_ptr<Delta>);
    bool trivial();

private:
    std::vector<std::unique_ptr<Delta>> deltas_;
};

class UndoStack {
public:
    UndoStack(unsigned int max_depth);
    void push(std::unique_ptr<DeltaFrame>);
    bool pop();
    void reset();

private:
    unsigned int max_depth_;
    unsigned int size_;
    std::deque<std::unique_ptr<DeltaFrame>> frames_;
};

class CreationDelta: public Delta {
public:
    CreationDelta(GameObject* object, RoomMap* room_map);
    void revert();

private:
    GameObject* object_;
    RoomMap* room_map_;
};

class DeletionDelta: public Delta {
public:
    DeletionDelta(std::unique_ptr<GameObject> object, RoomMap* room_map);
    void revert();

private:
    std::unique_ptr<GameObject> object_;
    RoomMap* room_map_;
};

class MotionDelta: public Delta {
public:
    MotionDelta(GameObject* object, Point p, RoomMap* room_map);
    void revert();

private:
    GameObject* object_;
    Point p_; // The previous position
    RoomMap* room_map_;
};

class AddLinkDelta: public Delta {
public:
    AddLinkDelta(Block* a, Block* b);
    void revert();

private:
    Block* a_;
    Block* b_;
};

class RemoveLinkDelta: public Delta {
public:
    RemoveLinkDelta(Block* a, Block* b);
    void revert();

private:
    Block* a_;
    Block* b_;
};

class DoorMoveDelta: public Delta {
public:
    DoorMoveDelta(RoomManager* mgr, Room* room, Point pos);
    void revert();

private:
    RoomManager* mgr_;
    Room* prev_room_;
    Point pos_;
};

#endif // DELTA_H
