#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

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

#include "common.h"

class Shader;
class DeltaFrame;
class WorldMap;

// The GameObject class will really act like the base class
// for Solid objects, until some non-solid objects exist too.
/** Abstract class for all objects that get placed on the map
 */
class GameObject {
public:
    GameObject(int x, int y);
    virtual ~GameObject() = 0;
    Layer layer() const;
    Point pos() const;
    virtual void draw(Shader*, int) = 0;
    virtual void cleanup(DeltaFrame*) = 0;
    virtual void reinit() = 0;
    // If not wall(), then downcast to Block is safe
    bool wall() const;

protected:
    Point pos_;
    bool wall_;
};

/** An immovable, static, Solid layer obstacle
 */
class Wall: public GameObject {
public:
    Wall(int x, int y);
    ~Wall();
    void draw(Shader*, int);
    void cleanup(DeltaFrame*);
    void reinit();
};

/** Abstract class for Solid objects which can move around and do things
 * Everything that's not a Wall is a Block
 */
class Block: public GameObject {
public:
    static BlockSet EMPTY_BLOCK_SET;

    Block(int x, int y);
    virtual ~Block() = 0;
    void set_car(bool car);
    // This draws an indication of the "car"-ness!  Call it in all subclasses
    void draw(Shader*, int);
    virtual const BlockSet& get_strong_links() = 0;
    virtual const BlockSet& get_weak_links() = 0;
    void set_pos(Point p, DeltaFrame*);
    void shift_pos(Point d, DeltaFrame*);
    BlockSet links();
    void add_link(Block*, DeltaFrame*);
    void remove_link(Block*, DeltaFrame*);
    void cleanup(DeltaFrame*);
    void reinit();


protected:
    bool car_;
    BlockSet links_;
};

enum class StickyLevel {
    None,
    Weak,
    Strong,
};

/** The standard type of block, represented by a grid aligned square
 */
class PushBlock: public Block {
public:
    static bool is_push_block(GameObject*);

    PushBlock(int x, int y);
    PushBlock(int x, int y, StickyLevel sticky);
    ~PushBlock();
    void set_sticky(StickyLevel sticky);
    void draw(Shader*, int);
    StickyLevel sticky();
    const BlockSet& get_strong_links();
    const BlockSet& get_weak_links();

private:
    StickyLevel sticky_;
};

/** A different type of block which forms "chains", represented by a diamond
 */
class SnakeBlock: public Block {
public:
    SnakeBlock(int x, int y);
    SnakeBlock(int x, int y, unsigned int ends);
    ~SnakeBlock();
    unsigned int ends();
    int distance();
    SnakeBlock* target();
    void set_distance(int);
    void set_target(SnakeBlock*);
    const BlockSet& get_strong_links();
    const BlockSet& get_weak_links();
    void draw(Shader*, int);

private:
    int ends_;
    int distance_;
    SnakeBlock* target_;
};

#endif // GAMEOBJECT_H
