#ifndef PLAYINGSTATE_H
#define PLAYINGSTATE_H

#include "common.h"
#include "delta.h"
#include "gamestate.h"
#include "gameobjectarray.h"

class GraphicsManager;
class Room;
class Player;
class MoveProcessor;

struct MapLocation;

class PlayingState: public GameState {
public:
    PlayingState(std::string name, Point3 pos, bool testing);
    virtual ~PlayingState();
    void init_player(Point3);
    void main_loop();
    void handle_input();
    bool activate_room(std::string);
    bool load_room(std::string);
    void use_door(MapLocation* dest);

private:
    std::map<std::string, std::unique_ptr<Room>> loaded_rooms_;
    std::unique_ptr<GameObjectArray> objs_;
    std::unique_ptr<MoveProcessor> move_processor_;
    std::unique_ptr<DeltaFrame> delta_frame_;
    Room* room_;
    Player* player_;
    UndoStack undo_stack_;

    bool testing_;

    friend DoorMoveDelta;
};

#endif // PLAYINGSTATE_H
