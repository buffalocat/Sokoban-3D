#include "block.h"
#include "graphicsmanager.h"
#include "delta.h"
#include "roommap.h"
#include "moveprocessor.h"
#include "component.h"
#include "mapfile.h"

#include "graphicsmanager.h"


Block::Block(Point3 pos, ColorCycle color, bool car):
GameObject(pos), comp_ {}, car_ {car}, color_ {color} {}

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

// Most block types don't form strong components
std::unique_ptr<StrongComponent> Block::make_strong_component(RoomMap* room_map) {
    auto comp = std::make_unique<SingletonComponent>(this);
    comp_ = comp.get();
    return std::move(comp);
}

// Some block types have trivial weak components, some don't
// Using sticky(), we can avoid rewriting this method
std::unique_ptr<WeakComponent> Block::make_weak_component(RoomMap* room_map) {
    auto comp = std::make_unique<WeakComponent>();
    comp_ = comp.get();
    if (sticky()) {
        std::vector<Block*> to_check {this};
        while (!to_check.empty()) {
            Block* cur = to_check.back();
            to_check.pop_back();
            comp->add_block(cur);
            for (Point3 d : DIRECTIONS) {
                Block* adj = dynamic_cast<Block*>(room_map->view(cur->shifted_pos(d)));
                if (adj && !adj->comp_ && adj->sticky() && color() == adj->color()) {
                    adj->comp_ = comp_;
                    to_check.push_back(adj);
                }
            }
        }
    } else {
        comp->add_block(this);
    }
    return std::move(comp);
}

StrongComponent* Block::s_comp() {
    return static_cast<StrongComponent*>(comp_);
}

WeakComponent* Block::w_comp() {
    return static_cast<WeakComponent*>(comp_);
}

void Block::reset_comp() {
    comp_ = nullptr;
}

void Block::set_z(int z) {
    pos_.z = z;
}

bool Block::sticky() {
    return false;
}

bool Block::car() {
    return car_;
}

void Block::draw(GraphicsManager* gfx) {
    Point3 p = pos_;
    gfx->set_model(glm::translate(glm::mat4(), glm::vec3(p.x, p.z, p.y)));
    gfx->set_color(COLORS[color()]);
    gfx->draw_cube();
}

void Block::get_weak_links(RoomMap*, std::vector<Block*>&) {}

bool Block::has_weak_neighbor(RoomMap* room_map) {
    if (!sticky()) {
        return false;
    }
    for (auto d : H_DIRECTIONS) {
        Block* adj = dynamic_cast<Block*>(room_map->view(shifted_pos(d)));
        if (adj && sticky() && color() == adj->color()) {
            return true;
        }
    }
    return false;
}

NonStickBlock::NonStickBlock(Point3 pos, ColorCycle color, bool car): Block(pos, color, car) {}

NonStickBlock::~NonStickBlock() {}

ObjCode NonStickBlock::obj_code() {
    return ObjCode::NonStickBlock;
}

GameObject* NonStickBlock::deserialize(MapFileI& file) {
    Point3 pos {file.read_point3()};
    ColorCycle color {file.read_color_cycle()};
    unsigned char b[1];
    file.read(b,1);
    return new NonStickBlock(pos, color, b[0]);
}

void NonStickBlock::draw(GraphicsManager* gfx) {
    gfx->set_tex(glm::vec2(2,0));
    Block::draw(gfx);
    gfx->set_tex(glm::vec2(0,0));
}

WeakBlock::WeakBlock(Point3 pos, ColorCycle color, bool car):
Block(pos, color, car) {}

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
    Point3 pos {file.read_point3()};
    ColorCycle color {file.read_color_cycle()};
    unsigned char b[1];
    file.read(b,1);
    return new WeakBlock(pos, color, b[0]);
}

void WeakBlock::draw(GraphicsManager* gfx) {
    gfx->set_tex(glm::vec2(1,0));
    Block::draw(gfx);
    gfx->set_tex(glm::vec2(0,0));
}


StickyBlock::StickyBlock(Point3 pos, ColorCycle color, bool car):
Block(pos, color, car) {}

StickyBlock::~StickyBlock() {}

ObjCode StickyBlock::obj_code() {
    return ObjCode::StickyBlock;
}

std::unique_ptr<StrongComponent> StickyBlock::make_strong_component(RoomMap* room_map) {
    auto comp = std::make_unique<ComplexComponent>();
    comp_ = comp.get();
    std::vector<StickyBlock*> to_check {this};
    while (!to_check.empty()) {
        StickyBlock* cur = to_check.back();
        to_check.pop_back();
        comp->add_block(cur);
        for (Point3 d : DIRECTIONS) {
            StickyBlock* adj = dynamic_cast<StickyBlock*>(room_map->view(cur->shifted_pos(d)));
            if (adj && !adj->comp_ && color() == adj->color()) {
                adj->comp_ = comp_;
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
    Point3 pos {file.read_point3()};
    ColorCycle color {file.read_color_cycle()};
    unsigned char b[1];
    file.read(b,1);
    return new StickyBlock(pos, color, b[0]);
}
