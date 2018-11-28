#ifndef COMPONENT_H
#define COMPONENT_H

#include "common.h"

class Block;
class RoomMap;
class DeltaFrame;

enum class MoveComponentState {
    Contingent = 1,
    Bad,
    Good,
};


class Component {
public:
    Component();
    virtual ~Component() = 0;

    virtual void reset_blocks_comps() = 0;
};


class StrongComponent: public Component {
public:
    StrongComponent();
    virtual ~StrongComponent() = 0;

    bool good();
    bool bad();
    void set_bad();
    void add_weak(StrongComponent*);
    virtual void add_block(Block*);
    virtual void add_push(StrongComponent*) = 0;
    virtual std::vector<Point3> to_push(Point3 d) = 0;
    virtual std::vector<Block*> get_weak_links(RoomMap*) = 0;
    virtual void resolve_contingent() = 0;
    virtual void collect_good(std::vector<Block*>&) = 0;
    const std::vector<Block*>& blocks();

protected:
    std::vector<StrongComponent*> weak_;
    MoveComponentState state_;
};

class ComplexComponent: public StrongComponent {
public:
    ComplexComponent();
    ~ComplexComponent();
    void add_block(Block*);
    void add_push(StrongComponent*);
    std::vector<Point3> to_push(Point3 d);
    std::vector<Block*> get_weak_links(RoomMap*);
    void resolve_contingent();
    void collect_good(std::vector<Block*>&);
    void reset_blocks_comps();

private:
    std::vector<Block*> blocks_;
    std::vector<StrongComponent*> push_;
};

class SingletonComponent: public StrongComponent {
public:
    SingletonComponent(Block* block);
    ~SingletonComponent();
    void add_push(StrongComponent*);
    std::vector<Point3> to_push(Point3 d);
    std::vector<Block*> get_weak_links(RoomMap*);
    void resolve_contingent();
    void collect_good(std::vector<Block*>&);
    void reset_blocks_comps();

protected:
    Block* block_;
    StrongComponent* push_;
};


class WeakComponent: public Component {
public:
    WeakComponent();
    ~WeakComponent();

    void add_block(Block*);
    bool falling();

    void check_land_first(RoomMap*);
    void settle_first();

    void collect_above(std::vector<Block*>& above_list, RoomMap* room_map);
    void collect_falling_unique(RoomMap* room_map);
    void reset_blocks_comps();
    bool drop_check(int layers_fallen, RoomMap* room_map, DeltaFrame* delta_frame);
    void check_land_sticky(int layers_fallen, RoomMap* room_map, DeltaFrame* delta_frame);
    void handle_unique_blocks(int layers_fallen, RoomMap* room_map, DeltaFrame* delta_frame);
    void settle(int layers_fallen, RoomMap* room_map, DeltaFrame* delta_frame);

private:
    std::vector<Block*> blocks_;
    std::vector<std::unique_ptr<GameObject>> unique_blocks_;
    std::vector<WeakComponent*> above_;
    bool falling_;
};

#endif // COMPONENT_H
