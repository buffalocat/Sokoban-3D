#ifndef ROOMMAP_H
#define ROOMMAP_H

#include <unordered_set>

#include "common.h"

class GameObjectArray;
class Signaler;
class Effects;
class MapLayer;
class GraphicsManager;
class DeltaFrame;
class MoveProcessor;
class GameObject;
class ObjectModifier;
class SnakeBlock;
class MapFileO;
class RoomMap;

typedef void(ObjectModifier::*MapCallback)(RoomMap*,DeltaFrame*);

class RoomMap {
public:
    RoomMap(GameObjectArray& objs, int width, int height, int depth);
    ~RoomMap();
    bool valid(Point3 pos);

    //void print_snakes();
    //void print_listeners();

    void push_full();
    void push_sparse();

    int& at(Point3);
    GameObject* view(Point3);

    void just_take(GameObject*);
    void just_put(GameObject*);
    void take(GameObject*);
    void put(GameObject*);
    void take_loud(GameObject*, DeltaFrame*);
    void put_loud(GameObject*, DeltaFrame*);

    void create(std::unique_ptr<GameObject>, DeltaFrame*);
    void create_abstract(std::unique_ptr<GameObject>, DeltaFrame*);
    void create_wall(Point3);
    void uncreate(GameObject*);
    void uncreate_abstract(GameObject*);
    void destroy(GameObject*, DeltaFrame*);
    void undestroy(GameObject*);

    void just_shift(GameObject*, Point3);
    void just_batch_shift(std::vector<GameObject*>, Point3);
    void shift(GameObject*, Point3, DeltaFrame*);
    void batch_shift(std::vector<GameObject*>, Point3, DeltaFrame*);

    void serialize(MapFileO& file) const;

    void draw(GraphicsManager*, float angle);
    void draw_layer(GraphicsManager*, int z);

    void set_initial_state(bool editor_mode);
    void reset_local_state();

    void initialize_automatic_snake_links();

    void push_signaler(std::unique_ptr<Signaler>);
    void check_signalers(DeltaFrame*, MoveProcessor*);
    void remove_signaler(Signaler*);
    void remove_obj_from_signalers(ObjectModifier*);

    void add_listener(ObjectModifier*, Point3);
    void remove_listener(ObjectModifier*, Point3);
    void activate_listeners_at(Point3);
    void activate_listener_of(ObjectModifier* obj);
    void alert_activated_listeners(DeltaFrame*, MoveProcessor*);

    void make_fall_trail(GameObject*, int height, int drop);

// Public "private" members
    int width_;
    int height_;
    int depth_;

    std::vector<GameObject*> agents_;

private:
    GameObjectArray& obj_array_;
    std::vector<std::unique_ptr<MapLayer>> layers_;

    std::unordered_map<Point3, std::vector<ObjectModifier*>, Point3Hash> listeners_;
    std::vector<std::unique_ptr<Signaler>> signalers_;

    std::unordered_set<ObjectModifier*> activated_listeners_;

    // TODO: find more appropriate place for this
    std::unique_ptr<Effects> effects_;

    // For providing direct signaler access
    friend class SwitchTab;
};

#endif // ROOMMAP_H
