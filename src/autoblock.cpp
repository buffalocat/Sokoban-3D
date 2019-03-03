#include "autoblock.h"

#include "gameobject.h"

AutoBlock::AutoBlock(GameObject* parent, RoomMap* room_map): ObjectModifier(parent), map_ {room_map} {}

AutoBlock::~AutoBlock() {}

std::string AutoBlock::name() {
    return "AutoBlock";
}

ModCode AutoBlock::mod_code() {
    return ModCode::AutoBlock;
}

void AutoBlock::serialize(MapFileO&) {}

void AutoBlock::deserialize(MapFileI& file, RoomMap* room_map, GameObject* parent) {
    parent->set_modifier(std::make_unique<AutoBlock>(parent, room_map));
}

std::unique_ptr<ObjectModifier> AutoBlock::duplicate(GameObject* parent, RoomMap*, DeltaFrame*) {
    return std::make_unique<AutoBlock>(parent, map_);
}
