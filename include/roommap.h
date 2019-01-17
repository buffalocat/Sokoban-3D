#ifndef ROOMMAP_H
#define ROOMMAP_H

#include "common.h"
#include "maplayer.h"
#include "switch.h"
#include "effects.h"

class GraphicsManager;
class DeltaFrame;
class GameObject;
class SnakeBlock;
class MapFileO;

class RoomMap {
public:
    RoomMap(int width, int height);
    ~RoomMap() = default;
    bool valid(Point3 pos);
    int width();
    int height();
    int depth();
    void push_full();
    void push_sparse();

    GameObject* view(Point3 pos);
    void take(Point3, DeltaFrame*);
    void take(GameObject*, DeltaFrame*);
    std::unique_ptr<GameObject> take_quiet(Point3);
    std::unique_ptr<GameObject> take_quiet(GameObject*);
    void put(std::unique_ptr<GameObject>, DeltaFrame*);
    void put_quiet(std::unique_ptr<GameObject>);

    void serialize(MapFileO& file) const;

    void draw(GraphicsManager*, float angle);
    void draw_layer(GraphicsManager*, int layer);

    void set_initial_state(bool editor_mode);

    void push_signaler(std::unique_ptr<Signaler>);
    void check_signalers(DeltaFrame*, std::vector<Block*>*);
    void remove_from_signalers(GameObject*);

    void make_fall_trail(Block*, int height, int drop);

private:
    int width_;
    int height_;
    std::vector<std::unique_ptr<MapLayer>> layers_;

    std::vector<std::unique_ptr<Signaler>> signalers_;
    std::unique_ptr<Effects> effects_;
};

#endif // ROOMMAP_H
