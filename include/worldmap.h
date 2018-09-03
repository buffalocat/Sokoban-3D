#ifndef WORLDMAP_H
#define WORLDMAP_H

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic ignored "-Wshadow"
#pragma GCC diagnostic ignored "-Wswitch-default"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wmissing-declarations"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#pragma GCC diagnostic pop

#include <vector>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <deque>

#include "shader.h"

const float BOARD_SIZE = 16.0f;


struct Point {
    int x;
    int y;
};

bool operator==(const Point& a, const Point& b);

struct PosHash {
    std::size_t operator()(const Point& p) const
    {
        return (p.x << 8) + p.y;
    }
};

typedef std::vector<std::pair<Point, unsigned int>> PosIdVec;
typedef std::unordered_map<Point, unsigned int, PosHash> PosIdMap;

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
    std::deque<std::unique_ptr<Delta>> deltas_;
};

class UndoStack {
public:
    UndoStack(unsigned int max_depth);
    void push(std::unique_ptr<DeltaFrame>);
    void pop(WorldMap*);

private:
    unsigned int max_depth_;
    unsigned int size_;
    std::deque<std::unique_ptr<DeltaFrame>> frames_;
};

class GameObject {
public:
    GameObject(int x, int y);
    virtual ~GameObject();
    unsigned int id() const;
    Point pos() const;
    void shift_pos(Point d, DeltaFrame*);
    virtual void draw(Shader*) = 0;
    PosIdMap const& get_strong_links();
    PosIdMap const& get_weak_links();
    void insert_strong_link(Point, unsigned int id);
    void insert_weak_link(Point, unsigned int id);
    bool sticky();
    bool weak_sticky();
    // This should be virtual, probably, but for now it's not
    Layer layer() const;
    virtual bool pushable() const = 0;
    static unsigned int GLOBAL_ID_COUNT;

protected:
    unsigned int gen_id();
    unsigned int id_;
    Point pos_;
    bool sticky_;
    bool weak_sticky_;
    PosIdMap sticky_links_;
    PosIdMap weak_sticky_links_;
};

class Car: public GameObject {
public:
    Car(int x, int y);
    ~Car();
    void draw(Shader*);
    Layer layer() const;
    bool pushable() const;
};

class Block: public GameObject {
public:
    Block(int x, int y);
    ~Block();
    void draw(Shader*);
    Layer layer() const;
    bool pushable() const;
};

class StickyBlock: public Block {
public:
    StickyBlock(int x, int y);
    void draw(Shader*);
};

class WeakStickyBlock: public Block {

};

class Wall: public GameObject {
public:
    Wall(int x, int y);
    ~Wall();
    void draw(Shader*);
    Layer layer() const;
    bool pushable() const;
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
    GameObject * view(Layer);
    GameObject * view_id(Layer, unsigned int id);
    void take(Layer, unsigned int id, DeltaFrame*);
    std::unique_ptr<GameObject> take_quiet(Layer, unsigned int id);
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
    GameObject* view_id(Point, Layer, unsigned int id);
    void take(Point, Layer, unsigned int id, DeltaFrame*);
    std::unique_ptr<GameObject> take_quiet(Point, Layer, unsigned int id);
    void put(std::unique_ptr<GameObject>, DeltaFrame*);
    void put_quiet(std::unique_ptr<GameObject>);
    void try_move(PosIdMap& to_move, Point dir, DeltaFrame* delta_frame);
    void draw(Shader*);
    void init_sticky();

private:
    int width_;
    int height_;
    std::vector<std::vector<MapCell>> map_;
};

#endif // WORLDMAP_H
