#ifndef COMPONENT_H
#define COMPONENT_H

#include "common.h"

class Block;
class RoomMap;

enum class ComponentState {
    Contingent = 1,
    Bad,
    Good,
};

class Component {
public:
    Component();
    virtual ~Component();

    bool good();
    bool bad();
    void set_bad();
    void add_weak(Component*);
    virtual void add_block(Block*);
    virtual void add_push(Component*) = 0;
    virtual std::vector<Point3> to_push(Point3 d) = 0;
    virtual std::vector<Block*> get_weak_links(RoomMap*) = 0;
    virtual void resolve_contingent() = 0;
    virtual void clean_up(std::vector<GameObject*>&) = 0;
    const std::vector<Block*>& blocks();

protected:
    ComponentState state_;
    std::vector<Component*> weak_;
};

class ComplexComponent: public Component {
public:
    ComplexComponent();
    ~ComplexComponent();
    void add_block(Block*);
    void add_push(Component*);
    std::vector<Point3> to_push(Point3 d);
    std::vector<Block*> get_weak_links(RoomMap*);
    void resolve_contingent();
    void clean_up(std::vector<GameObject*>&);

private:
    std::vector<Block*> blocks_;
    std::vector<Component*> push_;
};

class SingletonComponent: public Component {
public:
    SingletonComponent(Block* block);
    ~SingletonComponent();
    void add_push(Component*);
    std::vector<Point3> to_push(Point3 d);
    std::vector<Block*> get_weak_links(RoomMap*);
    void resolve_contingent();
    void clean_up(std::vector<GameObject*>&);

protected:
    Block* block_;
    Component* push_;
};

#endif // COMPONENT_H
