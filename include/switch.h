#ifndef SWITCH_H
#define SWITCH_H

#include <vector>

class RoomMap;
class DeltaFrame;
class Signaler;

// TODO: fix the modifier property hierarchy (Switch : ObjectModifier ???)
// For now, the only kind of switch is the pressure plate
class Switch {
public:
    Switch(bool persistent, bool active);
    virtual ~Switch();
    void push_signaler(Signaler*);
    virtual void check_send_signal(RoomMap*, DeltaFrame*) = 0;
    virtual bool should_toggle(RoomMap*) = 0;
    void toggle();

protected:
    bool persistent_;
    bool active_;
    std::vector<Signaler*> signalers_;
};
#endif // SWITCH_H
