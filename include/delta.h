#ifndef DELTA_H
#define DELTA_H

#include "common.h"

class RoomMap;
class GameObject;
class Block;
class PushBlock;
class SnakeBlock;
struct Point;
enum class Layer;

class Delta {
public:
    virtual void revert(RoomMap*) = 0;
    virtual ~Delta() {};
};

class DeltaFrame {
public:
    DeltaFrame();
    void revert(RoomMap*);
    void push(std::unique_ptr<Delta>);
    bool trivial();

private:
    std::vector<std::unique_ptr<Delta>> deltas_;
};

class UndoStack {
public:
    UndoStack(unsigned int max_depth);
    void push(std::unique_ptr<DeltaFrame>);
    void pop(RoomMap*);

private:
    unsigned int max_depth_;
    unsigned int size_;
    std::deque<std::unique_ptr<DeltaFrame>> frames_;
};

class CreationDelta: public Delta {
public:
    CreationDelta(GameObject* object);
    void revert(RoomMap*);

private:
    GameObject* object_;
};

class DeletionDelta: public Delta {
public:
    DeletionDelta(std::unique_ptr<GameObject>);
    void revert(RoomMap*);

private:
    std::unique_ptr<GameObject> object_;
};

class MotionDelta: public Delta {
public:
    MotionDelta(Block* object, Point p);
    void revert(RoomMap*);

private:
    Block* object_;
    Point p_; // The previous position
};

class AddLinkDelta: public Delta {
public:
    AddLinkDelta(Block* a, Block* b);
    void revert(RoomMap*);

private:
    Block* a_;
    Block* b_;
};

class RemoveLinkDelta: public Delta {
public:
    RemoveLinkDelta(Block* a, Block* b);
    void revert(RoomMap*);

private:
    Block* a_;
    Block* b_;
};

#endif // DELTA_H
