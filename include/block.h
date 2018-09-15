#ifndef BLOCK_H
#define BLOCK_H

#include "common.h"
#include "gameobject.h"

class GameObject;

/** Abstract class for Solid objects which can move around and do things
 * Everything that's not a Wall is a Block
 */
class Block: public GameObject {
public:
    static BlockSet EMPTY_BLOCK_SET;

    Block(int x, int y);
    Block(int x, int y, bool car);
    virtual ~Block() = 0;
    bool car();
    void set_car(bool car);
    void draw(Shader*, int);
    virtual const BlockSet& get_strong_links() = 0;
    virtual const BlockSet& get_weak_links() = 0;
    virtual bool push_recheck() = 0;
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
    PushBlock(int x, int y);
    PushBlock(int x, int y, bool car, StickyLevel sticky);
    ~PushBlock();
    ObjCode obj_code();
    void serialize(std::ofstream& file);
    bool push_recheck();
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
    SnakeBlock(int x, int y, bool car, unsigned int ends);
    ~SnakeBlock();
    ObjCode obj_code();
    void serialize(std::ofstream& file);
    bool push_recheck();
    unsigned int ends();
    int distance();
    SnakeBlock* target();
    void set_target(SnakeBlock*, int);
    void reset_target();
    const BlockSet& get_strong_links();
    const BlockSet& get_weak_links();
    void draw(Shader*, int);

private:
    int ends_;
    int distance_;
    SnakeBlock* target_;
};

#endif // BLOCK_H
