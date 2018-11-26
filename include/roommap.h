#ifndef ROOMMAP_H
#define ROOMMAP_H

#include "common.h"
#include "maplayer.h"

class GraphicsManager;
class DeltaFrame;
class GameObject;
class SnakeBlock;
class Switchable;
class Switch;
class MapFileO;

class RoomMap {
public:
    RoomMap(int width, int height);
    ~RoomMap() = default;
    bool valid(Point pos);
    int width() const;
    int height() const;
    void push_full();
    void push_sparse();

    GameObject* view(Point3 pos);
    void take(Point3, DeltaFrame*);
    std::unique_ptr<GameObject> take_quiet(Point3);
    std::unique_ptr<GameObject> take_quiet(GameObject*);
    void put(std::unique_ptr<GameObject>, DeltaFrame*);
    void put_quiet(std::unique_ptr<GameObject>);

    void serialize(MapFileO& file) const;

    void draw(GraphicsManager*);

    void set_initial_state(bool editor_mode);

private:
    int width_;
    int height_;
    std::vector<std::unique_ptr<MapLayer>> layers_;
};

#endif // ROOMMAP_H
