#ifndef SWITCH_H
#define SWITCH_H

#include <vector>
#include "objectmodifier.h"

class GameObject;
class RoomMap;
class DeltaFrame;
class Signaler;

class Switch: public ObjectModifier {
public:
    Switch(GameObject* parent, bool persistent, bool active);
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
