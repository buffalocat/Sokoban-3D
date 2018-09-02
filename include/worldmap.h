#ifndef WORLDMAP_H
#define WORLDMAP_H

#include <vector>
#include <memory>

struct Point {
    int x;
    int y;
};

Point negate(Point p);

enum class Layer {
    Floor,
    Player,
    Solid,
    COUNT,
};

class WorldMap;

class Delta {
public:
    virtual void revert(WorldMap*) = 0;
    virtual ~Delta() {};
};

class DeltaFrame {
public:
    DeltaFrame();
    void revert(WorldMap*);
    void push(std::unique_ptr<Delta>);
    bool trivial();

private:
    std::vector<std::unique_ptr<Delta>> deltas_;
};

class UndoStack {
public:
    UndoStack(int max_depth);

private:
    unsigned int max_depth_;
    unsigned int size_;
    std::vector<DeltaFrame> frames_;
};

class GameObject {
public:
    GameObject(int x, int y);
    unsigned int id() const;
    Layer layer() const;
    Point pos() const;
    void shift_pos(Point d, DeltaFrame*);

private:
    unsigned int id_;
    Layer layer_;
    Point pos_;
};

class CreationDelta: public Delta {
public:
    CreationDelta(GameObject const& object);
    void revert(WorldMap*);

private:
    Point pos_;
    Layer layer_;
    unsigned int id_;
};

class DeletionDelta: public Delta {
public:
    DeletionDelta(std::unique_ptr<GameObject>);
    void revert(WorldMap*);

private:
    std::unique_ptr<GameObject> object_;
};

class MotionDelta: public Delta {
public:
    MotionDelta(GameObject const& object, Point d);
    void revert(WorldMap*);

private:
    Point pos_; // "Current" (post-move) position
    Layer layer_;
    unsigned int id_;
    Point d_; // The amount we moved (as a 2-vector)
};

class MapCell {
public:
    MapCell();
    GameObject const* view(Layer);
    GameObject const* view_id(Layer, unsigned int id);
    void take(Layer, unsigned int id, DeltaFrame*);
    std::unique_ptr<GameObject> take_quiet(Layer, unsigned int id);
    void put(std::unique_ptr<GameObject>, DeltaFrame*);
    void put_quiet(std::unique_ptr<GameObject>);


private:
    std::array<std::vector<std::unique_ptr<GameObject>>, static_cast<unsigned int>(Layer::COUNT)> layers_;
};

class WorldMap {
public:
    WorldMap(int width, int height);
    bool valid(Point pos);
    GameObject const* view(Point, Layer);
    GameObject const* view_id(Point, Layer, unsigned int id);
    void take(Point, Layer, unsigned int id, DeltaFrame*);
    std::unique_ptr<GameObject> take_quiet(Point, Layer, unsigned int id);
    void put(std::unique_ptr<GameObject>, DeltaFrame*);
    void put_quiet(std::unique_ptr<GameObject>);
    void move_player(GameObject* player, Point dir);

private:
    int width_;
    int height_;
    std::vector<std::vector<MapCell>> map_;
};

#endif // WORLDMAP_H
