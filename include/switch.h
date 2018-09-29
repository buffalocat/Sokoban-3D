#ifndef SWITCH_H
#define SWITCH_H

#include "common.h"
#include "gameobject.h"

class RoomMap;
class DeltaFrame;

class Switchable {
public:
    Switchable(bool default_state, bool initial_state);
    virtual ~Switchable();
    void set_aw(bool active, bool waiting);
    bool state();
    virtual bool can_set_state(bool state, RoomMap*) = 0;
    void receive_signal(bool signal, RoomMap*, DeltaFrame*);
    void check_waiting(RoomMap*, DeltaFrame*);

protected:
    bool default_;

private:
    bool active_; // Opposite of default behavior
    bool waiting_; // Toggle active as soon as possible
};

class Gate: public GameObject, public Switchable {
public:
    Gate(int x, int y, bool def);
    ~Gate();
    ObjCode obj_code();
    Layer layer();
    void serialize(std::ofstream& file);
    static GameObject* deserialize(unsigned char* buffer);
    bool can_set_state(bool state, RoomMap*);

    void draw(Shader*);
};

class Signaler {
public:
    Signaler(unsigned int threshold, bool persistent);

    void push_switchable(Switchable*);
    void receive_signal(bool signal);
    void toggle();
    void check_send_signal(RoomMap*, DeltaFrame*);

private:
    unsigned int count_;
    unsigned int threshold_;
    bool state_;
    bool persistent_;
    std::vector<Switchable*> switchables_;
};

// For now, the only kind of switch is the pressure plate
class Switch: public GameObject {
public:
    Switch(int x, int y, unsigned char color, bool persistent);
    ObjCode obj_code();
    Layer layer();
    void serialize(std::ofstream& file);
    static GameObject* deserialize(unsigned char* buffer);

    void push_signaler(Signaler*);
    void check_send_signal(RoomMap*, DeltaFrame*, std::unordered_set<Signaler*>& check);
    bool should_toggle(RoomMap*);
    void toggle();

    void draw(Shader*);

private:
    unsigned char color_;
    bool persistent_;
    bool active_;
    std::vector<Signaler*> signalers_;
};

#endif // SWITCH_H
