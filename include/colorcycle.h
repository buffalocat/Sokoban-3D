#ifndef COLORCYCLE_H
#define COLORCYCLE_H

class MapFileO;
class Block;

class ColorCycle {
public:
    ColorCycle(unsigned char color);
    ColorCycle(unsigned char* b);
    ~ColorCycle();

private:
    unsigned char color();
    void insert_color(unsigned char color);
    void cycle(bool undo);
    // 5 is the maximum number of colors a block will have (probably)
    // This limit won't be stored in .maps, so it can be changed
    unsigned char color_[5];
    unsigned char size_;
    unsigned char index_;

    friend MapFileO;
    friend Block;
};

#endif // COLORCYCLE_H
