#ifndef PUSHBLOCK_H
#define PUSHBLOCK_H

#include "gameobject.h"
#include "common.h"

class PushBlock: public GameObject {
public:
    PushBlock(Point3 pos, int color, bool pushable, bool gravitable, Sticky sticky);
    virtual ~PushBlock();
    virtual void serialize(MapFileO& file) const;

    void collect_sticky_links(RoomMap*, Sticky sticky_level, std::vector<GameObject*>& links) const;
    //virtual bool has_weak_neighbor(RoomMap* room_map);

    void reset_animation();
    void set_linear_animation(Point3);
    void update_animation();
    void shift_pos_from_animation();
    FPoint3 real_pos();

    virtual void draw(GraphicsManager*);

    Sticky sticky_;
};

#endif // PUSHBLOCK_H
