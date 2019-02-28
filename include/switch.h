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
    void connect_to_signalers();
    virtual void check_send_signal(RoomMap*, DeltaFrame*) = 0;
    virtual bool should_toggle(RoomMap*) = 0;
    void toggle();

    virtual void cleanup_on_destruction(RoomMap* room_map);
    virtual void setup_on_undestruction(RoomMap* room_map);

protected:
    bool persistent_;
    bool active_;
    std::vector<Signaler*> signalers_;

    friend class ModifierTab;
};
#endif // SWITCH_H
