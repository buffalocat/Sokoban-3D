#ifndef PLAYINGSTATE_H
#define PLAYINGSTATE_H

#include "common.h"
#include "delta.h"
#include "gamestate.h"

class GraphicsManager;
class Room;
class Player;

struct MapLocation;

class PlayingState: public GameState {
public:
    PlayingState(std::string name, Point3 pos, bool testing);
    virtual ~PlayingState();
    void init_player(Point3);
    void main_loop();
    void handle_input(DeltaFrame*);
    bool activate_room(std::string);
    bool load_room(std::string);
    void use_door(MapLocation* dest, DeltaFrame*);

private:
    std::map<std::string, std::unique_ptr<Room>> loaded_rooms_;
    Room* room_;
    Player* player_;
    UndoStack undo_stack_;
    int keyboard_cooldown_;
    bool testing_;

    friend DoorMoveDelta;
};

#endif // PLAYINGSTATE_H
