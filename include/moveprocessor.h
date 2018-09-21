#ifndef MOVEPROCESSOR_H
#define MOVEPROCESSOR_H

#include "common.h"

class Block;
class RoomMap;
class DeltaFrame;
class SnakeBlock;

enum class LinkType {
    NONE, // Ensures that Strong isn't the "default"
    Strong,
    Weak,
    Push,
};

enum class ComponentState {
    Contingent,
    Bad,
    Good,
};

class Component {
public:
    Component();
    bool bad();
    bool good();
    void set_bad();
    bool check();
    void set_check(bool);
    void resolve_contingent();
    const std::vector<Block*>& blocks();
    void add_block(Block*);
    void add_push(Component*);
    void add_weak(Component*);

private:
    ComponentState state_;
    bool check_;
    std::vector<Block*> blocks_;
    std::vector<Component*> push_;
    std::vector<Component*> weak_;
};

class MoveProcessor {
public:
    MoveProcessor(RoomMap*, Point);
    void try_move(DeltaFrame*);
    Component* move_component(Block* block, bool recheck);
    void try_push(Component*, Point);
    void find_strong_component(Block*);

    void insert_touched_snake(SnakeBlock*);

private:
    RoomMap* map_;
    Point dir_;
    std::unordered_map<Block*, std::shared_ptr<Component>> comps_;
    // Sets for marking where interesting things happened
    std::unordered_set<Block*> maybe_broken_weak_;
    std::unordered_set<SnakeBlock*> touched_snakes_;
};

#endif // MOVEPROCESSOR_H
