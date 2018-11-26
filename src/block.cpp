#include "block.h"
#include "graphicsmanager.h"
#include "delta.h"
#include "roommap.h"
#include "moveprocessor.h"
#include "component.h"
#include "mapfile.h"

#include "graphicsmanager.h"


Block::Block(Point3 pos, unsigned char color, bool car):
GameObject(pos), comp_ {}, color_ {color}, car_ {car} {}

Block::~Block() {}

void Block::serialize(MapFileO& file) {
    file << color_;
    file << car_;
}

unsigned char Block::color() {
    return color_.color();
}

void Block::insert_color(unsigned char color) {
    color_.insert_color(color);
}

bool Block::cycle_color(bool undo) {
    if (color_.size_ == 1) {
        return false;
    }
    color_.cycle(undo);
    return true;
}

// Most blocks don't form strong components
std::unique_ptr<Component> Block::make_strong_component(RoomMap* room_map) {
    auto comp = std::make_unique<SingletonComponent>(this);
    comp_ = comp.get();
    return std::move(comp);
}

Component* Block::comp() {
    return comp_;
}

void Block::reset_comp() {
    comp_ = nullptr;
}

bool Block::sticky() {
    return false;
}

bool Block::car() {
    return car_;
}

void Block::get_weak_links(RoomMap*, std::vector<Block*>&) {}

NonStickBlock::NonStickBlock(Point3 pos, unsigned char color, bool is_car): Block(pos, color, is_car) {}

NonStickBlock::~NonStickBlock() {}

ObjCode NonStickBlock::obj_code() {
    return ObjCode::NonStickBlock;
}

GameObject* NonStickBlock::deserialize(MapFileI& file) {
    return nullptr;
}

void NonStickBlock::draw(GraphicsManager* gfx) {
    Point3 p = pos();
    gfx->set_model(glm::translate(glm::mat4(), glm::vec3(p.x, p.z, p.y)));
    gfx->set_color(COLORS[color()]);
    gfx->set_tex(glm::vec2(2,0));
    gfx->draw_cube();
    gfx->set_tex(glm::vec2(0,0));
    //Block::draw(gfx);
}

WeakBlock::WeakBlock(Point3 pos, unsigned char color, bool is_car):
Block(pos, color, is_car) {}

WeakBlock::~WeakBlock() {}

ObjCode WeakBlock::obj_code() {
    return ObjCode::WeakBlock;
}

bool WeakBlock::sticky() {
    return true;
}

void WeakBlock::get_weak_links(RoomMap* room_map, std::vector<Block*>& links) {
    for (Point3 d : DIRECTIONS) {
        Block* adj = dynamic_cast<Block*>(room_map->view(shifted_pos(d)));
        if (adj && adj->sticky() && color() == adj->color()) {
            links.push_back(adj);
        }
    }
}

GameObject* WeakBlock::deserialize(MapFileI& file) {
    return nullptr;
}

StickyBlock::StickyBlock(Point3 pos, unsigned char color, bool is_car):
Block(pos, color, is_car) {}

StickyBlock::~StickyBlock() {}

ObjCode StickyBlock::obj_code() {
    return ObjCode::StickyBlock;
}

std::unique_ptr<Component> StickyBlock::make_strong_component(RoomMap* room_map) {
    auto comp = std::make_unique<ComplexComponent>();
    comp_ = comp.get();
    std::vector<StickyBlock*> to_check {this};
    while (!to_check.empty()) {
        StickyBlock* cur = to_check.back();
        to_check.pop_back();
        comp_->add_block(cur);
        cur->comp_ = comp_;
        for (Point3 d : DIRECTIONS) {
            StickyBlock* adj = dynamic_cast<StickyBlock*>(room_map->view(cur->shifted_pos(d)));
            if (adj && !adj->comp_ && color() == adj->color()) {
                to_check.push_back(adj);
            }
        }
    }
    return std::move(comp);
}

void StickyBlock::get_weak_links(RoomMap* room_map, std::vector<Block*>& links) {
    for (Point3 d : DIRECTIONS) {
        // Two StickyBlocks can never have a weak link!
        WeakBlock* adj = dynamic_cast<WeakBlock*>(room_map->view(shifted_pos(d)));
        if (adj && color() == adj->color()) {
            links.push_back(adj);
        }
    }
}

bool StickyBlock::sticky() {
    return true;
}

GameObject* StickyBlock::deserialize(MapFileI& file) {
    return nullptr;
}
