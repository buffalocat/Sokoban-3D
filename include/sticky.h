#ifndef STICKY_H
#define STICKY_H

enum class Sticky : uint8_t {
    None = 0,
    Weak = 1,
    Strong = 2,
    All = 3,
};

Sticky operator &(Sticky a, Sticky b) {
    return static_cast<Sticky>(static_cast<uint8_t>(a) &
                               static_cast<uint8_t>(b));
}

#endif // STICKY_H
