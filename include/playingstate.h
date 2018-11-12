#ifndef PLAYINGSTATE_H
#define PLAYINGSTATE_H

#include "common.h"

#include "delta.h"
#include "gamestate.h"

class GraphicsManager;
class Room;
class Player;

class PlayingState: public GameState {
public:
    PlayingState(GraphicsManager*, std::string name, Point pos, bool testing);
    void init_player(Point);
    void main_loop();
    void handle_input(DeltaFrame*);
    bool activate_room(std::string);
    bool load_room(std::string);

private:
    std::map<std::string, std::unique_ptr<Room>> loaded_rooms_;
    Room* room_;
    Player* player_;
    UndoStack undo_stack_;
    int keyboard_cooldown_;
    bool testing_;
};

#endif // PLAYINGSTATE_H
