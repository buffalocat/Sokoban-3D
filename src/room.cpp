#include "room.h"

#include "roommap.h"
#include "camera.h"
#include "gameobject.h"

/*
Room::Room(std::string name, std::unique_ptr<RoomMap> room_map, std::unique_ptr<Camera> camera):
    name_ {name},
    map_ {std::move(room_map)},
    camera_ {std::move(camera)} {} //*/

Room::Room(std::string name, int w, int h): name_ {name},
    map_ {std::make_unique<RoomMap>(w, h)},
    camera_ {std::make_unique<Camera>(w, h)} {}

/*
RoomMap* Room::room_map() {
    return map_.get();
}

Camera* Room::camera() {
    return camera_.get();
}

std::string Room::name() {
    return name_;
} //*/

void Room::serialize(std::ofstream& file) {
    // Map initialization
    file << static_cast<unsigned char>(MapCode::SmallDims);
    file << static_cast<unsigned char>(map_->width());
    file << static_cast<unsigned char>(map_->height());

    map_->serialize(file);

    camera_->serialize(file);

    file << static_cast<unsigned char>(MapCode::End);
}
