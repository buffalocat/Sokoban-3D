#ifndef PLAYINGSTATE_H
#define PLAYINGSTATE_H

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

#include "gamestate.h"

class GameObjectArray;
class GraphicsManager;
class Room;
class GameObject;
class Player;
class MoveProcessor;

class UndoStack;
class DeltaFrame;
class DoorMoveDelta;

class Point3;

class Door;

class PlayingState: public GameState {
public:
    PlayingState(const std::string& name, Point3 pos, bool testing);
    virtual ~PlayingState();
    void init_player(Point3);
    void main_loop();
    void handle_input();
    bool activate_room(const std::string&);
    bool load_room(const std::string&);

    bool can_use_door(Door*, std::vector<GameObject*>&, bool* same_room);

private:
    std::unordered_map<std::string, std::unique_ptr<Room>> loaded_rooms_;
    std::unique_ptr<GameObjectArray> objs_;
    std::unique_ptr<MoveProcessor> move_processor_;
    std::unique_ptr<UndoStack> undo_stack_;
    std::unique_ptr<DeltaFrame> delta_frame_;
    Room* room_;
    Player* player_;

    bool testing_;

    friend DoorMoveDelta;
};

#endif // PLAYINGSTATE_H
