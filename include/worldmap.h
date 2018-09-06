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

    void reset_state();
    void move_solid(Point player_pos, Point dir, DeltaFrame* delta_frame);
    bool move_strong_component(PosIdMap& result, Point start_point, Point dir);

    void update_links(DeltaFrame*);
    void update_links_auxiliary(GameObject*, bool save_adj, DeltaFrame*);
    void set_initial_state();

    void draw(Shader*);

private:
    int width_;
    int height_;
    std::vector<std::vector<MapCell>> map_;

    // State variables
    PointSet seen_; // Points which have been inspected in a move
    PointSet not_move_; // Points guaranteed not to move during a move
    ObjSet moved_; // Objects which moved
    ObjSet link_update_; // Objects which didn't move but (may have) lost/formed links
    PointSet floor_update_; // Positions where an object (may have) left or entered the cell
};

#endif // WORLDMAP_H
