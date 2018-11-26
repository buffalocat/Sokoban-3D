#ifndef SWITCH_H
#define SWITCH_H

#include "common.h"
#include "gameobject.h"

class RoomMap;
class DeltaFrame;
class Switch;

class Switchable: public GameObject {
public:
    Switchable(Point3 pos, bool default_state, bool initial_state);
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

class Gate: public Switchable {
public:
    Gate(Point3 pos, bool def);
    ~Gate();
    ObjCode obj_code();
    void serialize(MapFileO& file);
    static GameObject* deserialize(MapFileI& file);
    bool can_set_state(bool state, RoomMap*);

    void draw(GraphicsManager*);
};

class Signaler {
public:
    Signaler(unsigned char threshold, bool persistent, bool active);
    ~Signaler();
    void push_switchable(Switchable*);
    void push_switch(Switch*);
    void receive_signal(bool signal);
    void toggle();
    void check_send_signal(RoomMap*, DeltaFrame*);

    void serialize(MapFileO& file);

private:
    unsigned char count_;
    unsigned char threshold_;
    bool active_;
    bool persistent_;
    std::vector<Switch*> switches_;
    std::vector<Switchable*> switchables_;
};

// For now, the only kind of switch is the pressure plate
class Switch: public GameObject {
public:
    Switch(Point3 pos, bool persistent, bool active);
    virtual ~Switch();
    void push_signaler(Signaler*);
    virtual void check_send_signal(RoomMap*, DeltaFrame*, std::unordered_set<Signaler*>& check) = 0;
    virtual bool should_toggle(RoomMap*) = 0;
    void toggle();

protected:
    bool persistent_;
    bool active_;
    std::vector<Signaler*> signalers_;
};

class PressSwitch: public Switch {
public:
    PressSwitch(Point3 pos, unsigned char color, bool persistent, bool active);
    ~PressSwitch();
    ObjCode obj_code();
    void serialize(MapFileO& file);
    static GameObject* deserialize(MapFileI& file);

    void check_send_signal(RoomMap*, DeltaFrame*, std::unordered_set<Signaler*>& check);
    bool should_toggle(RoomMap*);

    void draw(GraphicsManager*);

private:
    unsigned char color_;
};
#endif // SWITCH_H
