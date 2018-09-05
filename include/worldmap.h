#ifndef WORLDMAP_H
#define WORLDMAP_H

#include "common.h"

class Shader;
class DeltaFrame;
class GameObject;
enum class Layer;

typedef std::array<std::vector<std::unique_ptr<GameObject>>, static_cast<unsigned int>(Layer::COUNT)> MapCell;

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
    bool move_strong_component(PosIdMap& seen, PosIdMap& not_move, PosIdMap& result, ObjSet& broken, Point start_point, Point dir);

    void initialize_links(bool check_all);
    void init_sticky(); // Will be deprecated soon!!!

    void draw(Shader*);

private:
    int width_;
    int height_;
    std::vector<std::vector<MapCell>> map_;
    // State variables
    PointSet seen_;
};

#endif // WORLDMAP_H
