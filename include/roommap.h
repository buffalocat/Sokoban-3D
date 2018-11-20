#ifndef ROOMMAP_H
#define ROOMMAP_H

#include "common.h"

class GraphicsManager;
class DeltaFrame;
class GameObject;
class SnakeBlock;
class Switchable;
class Switch;

enum class Layer;

typedef std::vector<std::unique_ptr<GameObject>> MapCell;

class RoomMap {
public:
    RoomMap(int width, int height);
    ~RoomMap() = default;
    bool valid(Point pos);
    int width() const;
    int height() const;

    void serialize(std::ofstream& file) const;

    GameObject* view(Point, Layer);
    GameObject* view(Point, ObjCode);

    // Break the snake case convention here because of macros
    Switchable* view_Switchable(Point);
    Switch* view_Switch(Point);

    void take(GameObject*, DeltaFrame*);
    std::unique_ptr<GameObject> take_quiet(GameObject*);
    void put(std::unique_ptr<GameObject>, DeltaFrame*);
    void put_quiet(std::unique_ptr<GameObject>);

    void draw(GraphicsManager*);

    void set_initial_state(bool editor_mode);

    void print_contents();

private:
    int width_;
    int height_;
    std::vector<std::vector<MapCell>> map_;
};

#endif // ROOMMAP_H
