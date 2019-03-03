#ifndef PUSHBLOCK_H
#define PUSHBLOCK_H

#include "gameobject.h"

class PushBlock: public GameObject {
public:
    PushBlock(Point3 pos, int color, bool pushable, bool gravitable, Sticky sticky);
    virtual ~PushBlock();

    virtual std::string name();
    virtual ObjCode obj_code();
    virtual void serialize(MapFileO& file);
    static std::unique_ptr<GameObject> deserialize(MapFileI& file);

    void collect_sticky_links(RoomMap*, Sticky, std::vector<GameObject*>& links);

    virtual void draw(GraphicsManager*);

    Sticky sticky();

    Sticky sticky_;
};

#endif // PUSHBLOCK_H
