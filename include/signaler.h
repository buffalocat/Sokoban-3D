#ifndef SIGNALER_H
#define SIGNALER_H

#include <vector>

class Switchable;
class Switch;
class RoomMap;
class DeltaFrame;
class MoveProcessor;
class MapFileO;
class ObjectModifier;

class Signaler {
public:
    Signaler(unsigned char threshold, bool persistent, bool active);
    ~Signaler();

    void serialize(MapFileO& file);

    void push_switchable(Switchable*);
    void push_switch(Switch*);
    void receive_signal(bool signal);
    void toggle();
    void check_send_signal(RoomMap*, DeltaFrame*, MoveProcessor*);
    bool remove_object(ObjectModifier*);

private:
    unsigned char count_;
    unsigned char threshold_;
    bool active_;
    bool persistent_;
    std::vector<Switch*> switches_;
    std::vector<Switchable*> switchables_;
};

#endif // SIGNALER_H