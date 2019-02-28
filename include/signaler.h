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
    Signaler(int count, int threshold, bool persistent, bool active);
    ~Signaler();

    void serialize(MapFileO& file);

    void push_switchable(Switchable*);
    void push_switch(Switch*);
    void push_switchable_mutual(Switchable*);
    void push_switch_mutual(Switch*);
    void receive_signal(bool signal);
    void toggle();
    void check_send_signal(RoomMap*, DeltaFrame*, MoveProcessor*);
    bool remove_object(ObjectModifier*);

    std::vector<Switch*> switches_;
    std::vector<Switchable*> switchables_;

private:
    int count_;
    int threshold_;
    bool active_;
    bool persistent_;
};

#endif // SIGNALER_H
