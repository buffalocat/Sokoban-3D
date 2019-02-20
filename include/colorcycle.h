#ifndef COLORCYCLE_H
#define COLORCYCLE_H

class MapFileO;

class ColorCycle {
public:
    ColorCycle();
    ColorCycle(unsigned char color);
    ColorCycle(unsigned char* b);
    ~ColorCycle();

    unsigned char color();
    void insert_color(unsigned char color);
    // Returns whether the cycle had an effect (i.e., whether size_ > 1)
    bool cycle(bool undo);

private:
    // 5 is the maximum number of colors a block will have (probably)
    // This limit won't be stored in .maps, so it can be changed
    unsigned char color_[5];
    unsigned char size_;
    unsigned char index_;

    friend MapFileO;
};

#endif // COLORCYCLE_H
