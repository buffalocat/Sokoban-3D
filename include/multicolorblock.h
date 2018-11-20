#ifndef MULTICOLORBLOCK_H
#define MULTICOLORBLOCK_H

#include "common.h"
#include "block.h"

class TwoColorBlock {
public:
    TwoColorBlock(unsigned char alt_color);
    virtual ~TwoColorBlock();
    virtual void swap_colors() = 0;

protected:
    unsigned char alt_color_;
};


class TwoColorPushBlock: public PushBlock, public TwoColorBlock {
public:
    TwoColorPushBlock(int x, int y, unsigned char color, unsigned char alt_color, bool car, StickyLevel sticky);
    virtual ~TwoColorPushBlock();
    ObjCode obj_code();
    void serialize(std::ofstream& file);
    static GameObject* deserialize(unsigned char* buffer);
    void swap_colors();
};


#endif // MULTICOLORBLOCK_H
