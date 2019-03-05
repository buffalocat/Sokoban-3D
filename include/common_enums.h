#ifndef COMMON_ENUMS_H
#define COMMON_ENUMS_H

enum class MapCode {
    Dimensions = 1, // The dimensions of the room as 1 byte integers
    FullLayer = 2, // Create a new full layer
    SparseLayer = 3, // Create a new sparse layer
    DefaultPos = 4, // Mark the position to start the player at when loading from this map (only useful for testing, or a select few rooms)
    OffsetPos = 5, // The position that (0,0) was at when the room was created
    Objects = 6, // Read in all map objects
    CameraRects = 7, // Get a camera context rectangle
    SnakeLink = 8, // Link two snakes (1 = Right, 2 = Down)
    DoorDest = 9, // Give a door a destination Map + Pos
    Signaler = 10, // List of Switches and Switchables linked to a Signaler
    Walls = 11, // List of positions of walls
    PlayerData = 12, // Like Walls, the Player is listed separately from other objects
    GateBodyLocation = 13, // Indicates that a GateBody needs to be paired with its parent
    End = 255,
};

enum class ObjCode {
    NONE = 0,
    PushBlock = 1,
    SnakeBlock = 2,
    Wall = 3,
    Player = 4,
    GateBody = 5,
};

enum class ModCode {
    NONE = 0,
    Door = 1,
    Car = 2,
    PressSwitch = 3,
    Gate = 4,
    AutoBlock = 5,
};

enum class CameraCode {
    NONE = 0,
    Free = 1,
    Fixed = 2,
    Clamped = 3,
    Null = 4,
};

enum class Sticky {
    None = 0,
    Weak = 1,
    Strong = 2,
    AllStick = 3,
    Snake = 4,
    SnakeWeak = 5,
    All = 7,
};

Sticky operator &(Sticky a, Sticky b);

enum class RidingState {
    Free = 1,
    Bound = 2,
    Riding = 3,
};

#endif // COMMON_ENUMS_H
