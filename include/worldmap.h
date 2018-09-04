#ifndef WORLDMAP_H
#define WORLDMAP_H

#include "common.h"

class Shader;
class DeltaFrame;
class GameObject;
enum class Layer;

class MapCell {
public:
    MapCell();
    GameObject* view(Layer);
    void take(Layer, DeltaFrame*);
    void take_id(Layer, GameObject*, DeltaFrame*);
    std::unique_ptr<GameObject> take_quiet(Layer);
    std::unique_ptr<GameObject> take_quiet_id(Layer, GameObject*);
    void put(std::unique_ptr<GameObject>, DeltaFrame*);
    void put_quiet(std::unique_ptr<GameObject>);
    void draw(Shader*);

private:
    std::array<std::vector<std::unique_ptr<GameObject>>, static_cast<unsigned int>(Layer::COUNT)> layers_;
};

class WorldMap {
public:
    WorldMap(int width, int height);
    bool valid(Point pos);
    GameObject* view(Point, Layer);
    void take(Point, Layer, DeltaFrame*);
    void take_id(Point, Layer, GameObject*, DeltaFrame*);
    std::unique_ptr<GameObject> take_quiet(Point, Layer);
    std::unique_ptr<GameObject> take_quiet_id(Point, Layer, GameObject*);
    void put(std::unique_ptr<GameObject>, DeltaFrame*);
    void put_quiet(std::unique_ptr<GameObject>);
    void move_solid(Point player_pos, Point dir, DeltaFrame* delta_frame);
    bool move_strong_component(PosIdMap& seen, PosIdMap& not_move, PosIdMap& result, Point start_point, Point dir);
    void draw(Shader*);
    void init_sticky();

private:
    int width_;
    int height_;
    std::vector<std::vector<MapCell>> map_;
};

#endif // WORLDMAP_H
