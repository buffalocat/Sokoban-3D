#ifndef MAPLAYER_H
#define MAPLAYER_H

#include "common.h"

class GameObject;
class DeltaFrame;
class RoomMap;
class MapFileO;
class GraphicsManager;

class MapLayer {
public:
    MapLayer(RoomMap*);
    virtual ~MapLayer();

    virtual GameObject* view(Point) = 0;
    virtual void take(Point, DeltaFrame*) = 0;
    virtual std::unique_ptr<GameObject> take_quiet(Point) = 0;
    virtual void put(std::unique_ptr<GameObject>, DeltaFrame*) = 0;
    virtual void put_quiet(std::unique_ptr<GameObject>) = 0;
    virtual MapCode type() = 0;
    virtual void serialize(MapFileO&, std::vector<GameObject*>&) = 0;
    virtual void draw(GraphicsManager*) = 0;

protected:
    RoomMap* parent_map_;
};


class FullMapLayer: public MapLayer {
public:
    FullMapLayer(RoomMap*, int width, int height);
    ~FullMapLayer();

    GameObject* view(Point);
    void take(Point, DeltaFrame*);
    std::unique_ptr<GameObject> take_quiet(Point);
    void put(std::unique_ptr<GameObject>, DeltaFrame*);
    void put_quiet(std::unique_ptr<GameObject>);
    MapCode type();
    void serialize(MapFileO&, std::vector<GameObject*>&);
    void draw(GraphicsManager*);

private:
    std::vector<std::vector<std::unique_ptr<GameObject>>> map_;
};


class SparseMapLayer: public MapLayer {
public:
    SparseMapLayer(RoomMap*);
    ~SparseMapLayer();

    GameObject* view(Point);
    void take(Point, DeltaFrame*);
    std::unique_ptr<GameObject> take_quiet(Point);
    void put(std::unique_ptr<GameObject>, DeltaFrame*);
    void put_quiet(std::unique_ptr<GameObject>);
    MapCode type();
    void serialize(MapFileO&, std::vector<GameObject*>&);
    void draw(GraphicsManager*);

private:
    std::unordered_map<Point, std::unique_ptr<GameObject>, PointHash> map_;
};


#endif // MAPLAYER_H
