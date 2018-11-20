#ifndef SWITCH_H
#define SWITCH_H

#include "common.h"
#include "gameobject.h"

class RoomMap;
class DeltaFrame;
class Switch;

class Switchable: public GameObject {
public:
    Switchable(int x, int y, bool default_state, bool initial_state);
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
    Gate(int x, int y, bool def);
    ~Gate();
    ObjCode obj_code();
    Layer layer();
    void serialize(std::ofstream& file);
    static GameObject* deserialize(unsigned char* buffer);
    bool can_set_state(bool state, RoomMap*);

    void draw(GraphicsManager*);
};

class Signaler {
public:
    Signaler(unsigned int threshold, bool persistent, bool active);
    ~Signaler();
    void push_switchable(Switchable*);
    void push_switch(Switch*);
    void receive_signal(bool signal);
    void toggle();
    void check_send_signal(RoomMap*, DeltaFrame*);

    void serialize(std::ofstream& file);

private:
    unsigned int count_;
    unsigned int threshold_;
    bool active_;
    bool persistent_;
    std::vector<Switch*> switches_;
    std::vector<Switchable*> switchables_;
};

// For now, the only kind of switch is the pressure plate
class Switch: public GameObject {
public:
    Switch(int x, int y, bool persistent, bool active);
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
    PressSwitch(int x, int y, unsigned char color, bool persistent, bool active);
    ~PressSwitch();
    ObjCode obj_code();
    Layer layer();
    void serialize(std::ofstream& file);
    static GameObject* deserialize(unsigned char* buffer);

    void check_send_signal(RoomMap*, DeltaFrame*, std::unordered_set<Signaler*>& check);
    bool should_toggle(RoomMap*);

    void draw(GraphicsManager*);

private:
    unsigned char color_;
};
#endif // SWITCH_H
