#ifndef SWITCHABLE_H
#define SWITCHABLE_H

#include <vector>

class RoomMap;
class DeltaFrame;
class GameObject;

class Switchable {
public:
    Switchable(bool default_state, bool initial_state);
    virtual ~Switchable();
    void set_aw(bool active, bool waiting, RoomMap*);
    bool state();
    virtual bool can_set_state(bool state, RoomMap*) = 0;
    void receive_signal(bool signal, RoomMap*, DeltaFrame*, std::vector<GameObject*>&);
    virtual void apply_state_change(RoomMap*, std::vector<GameObject*>&);
    void check_waiting(RoomMap*, DeltaFrame*);

protected:
    bool default_;

private:
    bool active_; // Opposite of default behavior
    bool waiting_; // Toggle active as soon as possible
};


#endif // SWITCHABLE_H
