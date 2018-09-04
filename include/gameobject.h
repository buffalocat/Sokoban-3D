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

// The GameObject class will really act like the base class
// for Solid objects, until some non-solid objects exist too.
/** Abstract class for all objects that get placed on the map
 */
class GameObject {
public:
    static unsigned int GLOBAL_ID_COUNT;
    static PosIdMap EMPTY_POS_ID_MAP;

    GameObject(int x, int y);
    virtual ~GameObject() = 0;
    unsigned int id() const;
    Layer layer() const;
    Point pos() const;
    virtual void draw(Shader*) = 0;
    // If not wall(), then downcast to Block is safe
    bool wall() const;

protected:
    unsigned int gen_id();
    unsigned int id_;
    Point pos_;
    bool wall_;
};

/** An immovable, static, Solid layer obstacle
 */
class Wall: public GameObject {
public:
    Wall(int x, int y);
    ~Wall();
    void draw(Shader*);
};

// This is just for downcasting purposes
// At the moment I only have these two in mind, but there could easily be more
enum class BlockType {
    Push,
    Snake,
};

/** Abstract class for Solid objects which can move around and do things
 * Everything that's not a Wall is a Block
 */
class Block: public GameObject {
public:
    Block(int x, int y);
    virtual ~Block() = 0;
    void set_car(bool car);
    // This draws an indication of the "car"-ness!  Call it in all subclasses
    void draw(Shader*);
    BlockType type();
    virtual PosIdMap& get_strong_links() = 0;
    virtual PosIdMap& get_weak_links() = 0;
    void shift_pos(Point d, DeltaFrame*);

protected:
    BlockType type_;
    bool car_;
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
    PushBlock(int x, int y);
    PushBlock(int x, int y, StickyLevel sticky);
    ~PushBlock();
    void set_sticky(StickyLevel sticky);
    void draw(Shader*);
    StickyLevel sticky();
    PosIdMap& get_strong_links();
    PosIdMap& get_weak_links();
    void insert_link(PosId);

private:
    StickyLevel sticky_;
    PosIdMap links_;
};

/** A different type of block which forms "chains", represented by a diamond
 */
class SnakeBlock: public Block {
    SnakeBlock(int x, int y);
    void draw(Shader*);
};

#endif // GAMEOBJECT_H
