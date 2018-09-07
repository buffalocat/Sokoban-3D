#ifndef WORLDMAP_H
#define WORLDMAP_H

#include "common.h"

class Shader;
class DeltaFrame;
class GameObject;
class SnakeBlock;

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
    void update_links_auxiliary(GameObject*, DeltaFrame*);
    void pull_snakes(DeltaFrame*);
    void pull_snakes_auxiliary(SnakeBlock*, DeltaFrame*);
    void set_initial_state();
    void update_snakes(DeltaFrame*);
    void snake_split(SnakeBlock*, SnakeBlock*, SnakeBlock*);
    void snake_split_reverse(SnakeBlock*, SnakeBlock*, SnakeBlock*);

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
    std::unordered_set<SnakeBlock*> snakes_; // All snakes
    std::unordered_set<SnakeBlock*> pushed_snakes_; // Snakes that were directly pushed
};

#endif // WORLDMAP_H
