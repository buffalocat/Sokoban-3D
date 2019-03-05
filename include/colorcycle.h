#ifndef COLORCYCLE_H
#define COLORCYCLE_H

#include "common_constants.h"

class ColorCycle {
public:
    ColorCycle();
    ColorCycle(int color);
    ColorCycle(unsigned char* b);
    ~ColorCycle();

    int color();
    void set_current(int color);
    // Returns whether the cycle had an effect (i.e., whether size_ > 1)
    bool cycle(bool undo);

private:
    // The limit on number of colors is somewhat arbitrary
    // This limit won't be stored in .maps, so it can be changed
    int index_;
    int size_;
    int color_[MAX_COLOR_CYCLE];

    friend class MapFileO;
    friend class ModifierTab;
};

#endif // COLORCYCLE_H
