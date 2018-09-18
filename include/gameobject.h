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
    virtual ~GameObject() = 0;
    virtual ObjCode obj_code() = 0;
    virtual void serialize(std::ofstream& file) = 0;
    Layer layer() const;
    Point pos() const;
    Point shifted_pos(Point) const;
    virtual void draw(Shader*, int) = 0;
    virtual void cleanup(DeltaFrame*) = 0;
    virtual void reinit() = 0;
    // If not wall(), then downcast to Block is safe
    bool wall() const;

protected:
    GameObject(int x, int y);
    Point pos_;
    bool wall_;
};

/** An immovable, static, Solid layer obstacle
 */
class Wall: public GameObject {
public:
    Wall(int x, int y);
    ~Wall();
    ObjCode obj_code();
    void serialize(std::ofstream& file);
    void draw(Shader*, int);
    void cleanup(DeltaFrame*);
    void reinit();
};

#endif // GAMEOBJECT_H
