#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#include <vector>
#include <memory>

// Later we won't need this, but for now it's convenient for draw_force_indicators()
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic ignored "-Wshadow"
#pragma GCC diagnostic ignored "-Wswitch-default"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wmissing-declarations"

#include <glm/glm.hpp>

#pragma GCC diagnostic pop

#include "common_enums.h"
#include "point.h"

class ObjectModifier;
class PositionalAnimation;
class DeltaFrame;
class RoomMap;
class GraphicsManager;
class MapFileI;
class MapFileO;

struct Component;
struct PushComponent;
struct FallComponent;

// Base class of all objects that occupy a tile in a RoomMap
class GameObject {
public:
    virtual ~GameObject();
    GameObject(const GameObject&);

    std::string to_str();
    virtual std::string name() = 0;

    virtual ObjCode obj_code() = 0;
    virtual bool skip_serialization();
    virtual void serialize(MapFileO& file) = 0;
    virtual bool relation_check();
    virtual void relation_serialize(MapFileO& file);

    virtual bool is_agent();

    Point3 shifted_pos(Point3 d);
    void abstract_shift(Point3 dpos, DeltaFrame* delta_frame);

    virtual void draw(GraphicsManager*) = 0;
    void draw_force_indicators(GraphicsManager*, glm::mat4& model);

    virtual void setup_on_put(RoomMap*);
    virtual void cleanup_on_take(RoomMap*);

    virtual void setup_on_undestruction(RoomMap*);
    virtual void cleanup_on_destruction(RoomMap*);

    PushComponent* push_comp();
    FallComponent* fall_comp();

    void collect_sticky_component(RoomMap*, Sticky, Component*);
    virtual Sticky sticky() = 0;
    bool has_sticky_neighbor(RoomMap*);
    virtual void collect_sticky_links(RoomMap*, Sticky, std::vector<GameObject*>& links) = 0;
    virtual void collect_special_links(RoomMap*, Sticky, std::vector<GameObject*>& links);

    void set_modifier(std::unique_ptr<ObjectModifier> mod);
    ObjectModifier* modifier();

    void reset_animation();
    void set_linear_animation(Point3);
    bool update_animation(); // Return whether the animation is done
    void shift_pos_from_animation();
    FPoint3 real_pos();

    std::unique_ptr<ObjectModifier> modifier_;
    std::unique_ptr<PositionalAnimation> animation_;
    Component* comp_;
    Point3 pos_;
    int id_;
    int color_;
    bool pushable_;
    bool gravitable_;

    bool tangible_;

protected:
    GameObject(Point3 pos, int color, bool pushable, bool gravitable);
};

#endif // GAMEOBJECT_H
