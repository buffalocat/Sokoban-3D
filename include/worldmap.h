#ifndef WORLDMAP_H
#define WORLDMAP_H

#include "common.h"

#include <fstream>

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
    int width() const;
    int height() const;
    const std::vector<Block*>& movers();
    Block* prime_mover();
    void add_mover(Block*);

    void serialize(std::ofstream& file) const;

    GameObject* view(Point, Layer);
    void take(Point, Layer, DeltaFrame*);
    void take_id(Point, Layer, GameObject*, DeltaFrame*);
    std::unique_ptr<GameObject> take_quiet(Point, Layer);
    std::unique_ptr<GameObject> take_quiet_id(Point, Layer, GameObject*);
    void put(std::unique_ptr<GameObject>, DeltaFrame*);
    void put_quiet(std::unique_ptr<GameObject>);

    void draw(Shader*);

private:
    int width_;
    int height_;
    std::vector<std::vector<MapCell>> map_;

    // State variables
    std::vector<Block*> movers_;
};

#endif // WORLDMAP_H
