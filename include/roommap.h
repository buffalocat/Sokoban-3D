#ifndef ROOMMAP_H
#define ROOMMAP_H

#include "gameobjectarray.h"
#include "common.h"
#include "maplayer.h"
#include "switch.h"
#include "effects.h"

class GraphicsManager;
class DeltaFrame;
class GameObject;
class SnakeBlock;
class MapFileO;
class RoomMap;

typedef void(GameObject::*MapCallback)(RoomMap*,DeltaFrame*);

class RoomMap {
public:
    RoomMap(GameObjectArray& objs, int width, int height, int depth);
    ~RoomMap() = default;
    bool valid(Point3 pos);

    void push_full();
    void push_sparse();

    int& at(Point3);

    GameObject* view(Point3 pos);
    void take(GameObject*, DeltaFrame*);
    void put(GameObject*, DeltaFrame*);
    void create(std::unique_ptr<GameObject>, Point3, DeltaFrame*);
    void destroy(Point3, DeltaFrame*);

    void serialize(MapFileO& file) const;

    void draw(GraphicsManager*, float angle);

    void set_initial_state(bool editor_mode);
    void reset_local_state();

    void push_signaler(std::unique_ptr<Signaler>);
    void check_signalers(DeltaFrame*, std::vector<Block*>*);
    void remove_from_signalers(GameObject*);

    void make_fall_trail(Block*, int height, int drop);

// Public "private" members
    int width_;
    int height_;
    int depth_;

private:
    GameObjectArray& obj_array_;
    std::vector<std::unique_ptr<MapLayer>> layers_;

    std::unordered_map<Point3, std::vector<std::pair<GameObject*, MapCallback>>, Point3Hash> listeners_;
    std::vector<std::unique_ptr<Signaler>> signalers_;

    std::unordered_set<GameObject*> activated_listeners_;
    std::unordered_set<SnakeBlock*> snakes_to_update_;

    // TODO: find more appropriate place for this
    std::unique_ptr<Effects> effects_;
};

#endif // ROOMMAP_H
