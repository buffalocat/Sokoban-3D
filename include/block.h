#ifndef BLOCK_H
#define BLOCK_H

#include "common.h"
#include "gameobject.h"

class GameObject;
class MoveProcessor;

/** Abstract class for Solid objects which can move around and do things
 * Everything that's not a Wall is a Block
 */
class Block: public GameObject {
public:
    static BlockSet EMPTY_BLOCK_SET;

    Block(int x, int y);
    Block(int x, int y, bool car);
    virtual ~Block() = 0;
    Layer layer();
    bool car();
    void set_car(bool car);
    void draw(Shader*);
    virtual bool push_recheck(MoveProcessor*) = 0;
    virtual const BlockSet& get_strong_links() = 0;
    virtual const BlockSet& get_weak_links() = 0;
    void set_pos(Point p);
    void shift_pos(Point d);
    void add_link(Block*, DeltaFrame*);
    void remove_link(Block*, DeltaFrame*);
    void remove_all_links(DeltaFrame*);
    void cleanup(DeltaFrame*);
    void reinit();
    void check_remove_local_links(RoomMap*, DeltaFrame*);
    virtual void check_add_local_links(RoomMap*, DeltaFrame*) = 0;
    virtual void post_move_reset();

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
    static GameObject* deserialize(unsigned char* buffer);
    bool relation_check();
    void relation_serialize(std::ofstream& file);

    bool push_recheck(MoveProcessor*);
    void set_sticky(StickyLevel sticky);
    void draw(Shader*);
    StickyLevel sticky();
    const BlockSet& get_strong_links();
    const BlockSet& get_weak_links();
    void check_add_local_links(RoomMap*, DeltaFrame*);

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
    static GameObject* deserialize(unsigned char* buffer);
    bool relation_check();
    void relation_serialize(std::ofstream& file);

    bool push_recheck(MoveProcessor*);
    unsigned int ends();
    int distance();
    void reset_target();
    const BlockSet& get_strong_links();
    const BlockSet& get_weak_links();
    void draw(Shader*);
    bool available();
    bool confused(RoomMap*);
    void check_add_local_links(RoomMap*, DeltaFrame*);
    void collect_unlinked_neighbors(RoomMap*, std::unordered_set<SnakeBlock*>&);
    void pull(RoomMap*, DeltaFrame*, std::unordered_set<SnakeBlock*>&, Point);
    void pull_aux(RoomMap*, DeltaFrame*, std::unordered_set<SnakeBlock*>&, Point);
    void post_move_reset();

private:
    unsigned int ends_;
    int distance_;
    SnakeBlock* target_;
};

#endif // BLOCK_H
