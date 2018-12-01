#include "snakeblock.h"
#include "graphicsmanager.h"
#include "mapfile.h"
#include "component.h"

#include <algorithm>

SnakeBlock::SnakeBlock(Point3 pos, ColorCycle color, bool is_car, unsigned char ends):
Block(pos, color, is_car), links_ {}, ends_ {ends} {}

SnakeBlock::~SnakeBlock() {}

ObjCode SnakeBlock::obj_code() {
    return ObjCode::SnakeBlock;
}

void SnakeBlock::draw(GraphicsManager* gfx) {
    Point3 p = pos();
    glm::mat4 model = glm::translate(glm::mat4(), glm::vec3(p.x, 0.5f, p.y));
    model = glm::scale(model, glm::vec3(0.7071f, 1, 0.7071f));
    model = glm::rotate(model, glm::radians(45.0f), glm::vec3(0, 1, 0));
    gfx->set_model(model);
    gfx->set_color(COLORS[color()]);
    if (ends_ == 1) {
        gfx->set_tex(glm::vec2(2,0));
        gfx->draw_cube();
        gfx->set_tex(glm::vec2(0,0));
    } else {
        gfx->draw_cube();
    }
}

void SnakeBlock::serialize(MapFileO& file) {
    Block::serialize(file);
    file << ends_;
}

GameObject* SnakeBlock::deserialize(MapFileI& file) {
    Point3 pos {file.read_point3()};
    ColorCycle color {file.read_color_cycle()};
    unsigned char b[2];
    file.read(b, 2);
    return new SnakeBlock(pos, color, b[0], b[1]);
}

bool SnakeBlock::relation_check() {
    return true;
}

void SnakeBlock::relation_serialize(MapFileO& file) {
    unsigned char link_encode = 0;
    for (auto& link : links_) {
        Point3 q = link->pos();
        // Snake links are always adjacent, and we only bother to
        // record links to the Right or Down
        if (q.x > pos_.x) {
            ++link_encode;
        } else if (q.y > pos_.y) {
            link_encode += 2;
        }
    }
    if (link_encode) {
        file << MapCode::SnakeLink;
        file << pos_;
        file << link_encode;
    }
}

std::unique_ptr<StrongComponent> SnakeBlock::make_strong_component(RoomMap* room_map) {
    auto unique_comp = std::make_unique<SnakeComponent>(this);
    comp_ = unique_comp.get();
    return std::move(unique_comp);
}

std::unique_ptr<WeakComponent> SnakeBlock::make_weak_component(RoomMap* room_map) {
    auto unique_comp = std::make_unique<WeakComponent>();
    comp_ = unique_comp.get();
    std::vector<Block*> to_check {this};
    while (!to_check.empty()) {
        Block* cur = to_check.back();
        to_check.pop_back();
        comp_->add_block(cur);
        for (auto link : links_) {
            if (!link->comp_) {
                link->comp_ = comp_;
                to_check.push_back(link);
            }
        }
    }
    return std::move(unique_comp);
}

void SnakeBlock::root_init() {
    if (links_.size() == 2) {
        Point3 p = links_[0]->pos_;
        Point3 q = links_[1]->pos_;
        if ((p.x + q.x == 2 * pos_.x) && (p.y + q.y == 2 * pos_.y)) {
            comp_->set_pushed();
        }
    }
}

bool SnakeBlock::in_links(SnakeBlock* sb) {
    return links_.find(links_.begin(), links_.end(), sb) != links_.end();
}

void SnakeBlock::add_link(SnakeBlock* sb, DeltaFrame* delta_frame) {
    links_.push_back(sb);
    sb->links_.push_back(this);
    if (delta_frame) {
        delta_frame->push(std::make_unique<AddLinkDelta>(this, sb));
    }
}

void SnakeBlock::remove_link(SnakeBlock* sb, DeltaFrame* delta_frame) {
    links_.remove(sb);
    sb->links_.remove(this);
    if (delta_frame) {
        delta_frame->push(std::make_unique<RemoveLinkDelta>(this, sb));
    }
}

void SnakeBlock::check_local_links(RoomMap* room_map, DeltaFrame* delta_frame) {
    // Remove expired links!
    if (!available() || confused(room_map)) {
        return;
    }
    for (auto& d : H_DIRECTIONS) {
        auto snake = dynamic_cast<SnakeBlock*>(room_map->view(shifted_pos(d)));
        if (snake && color_ == snake->color_ && snake->available() && !snake->confused(room_map)) {
            add_link(snake, delta_frame);
        }
    }
}

bool SnakeBlock::available() {
    return links_.size() < ends_;
}

bool SnakeBlock::confused(RoomMap* room_map) {
    unsigned int available_count = 0;
    for (auto& d : H_DIRECTIONS) {
        auto snake = dynamic_cast<SnakeBlock*>(room_map->view(shifted_pos(d)));
        if (snake && color_ == snake->color_ && (snake->available() || links_.count(snake))) {
            ++available_count;
        }
    }
    return available_count > ends_;
}

void SnakeBlock::collect_unlinked_neighbors(RoomMap* room_map, std::unordered_set<SnakeBlock*>& check_snakes) {
    for (auto& d : H_DIRECTIONS) {
        auto snake = dynamic_cast<SnakeBlock*>(room_map->view(shifted_pos(d)));
        if (snake && snake->available()) {
            check_snakes.insert(snake);
        }
    }
}


SnakePuller::SnakePuller(RoomMap* room_map, DeltaFrame* delta_frame, Point dir):
room_map_ {room_map}, delta_frame_ {delta_frame}, dir_ {dir} {}

SnakePuller::~SnakePuller() {}

void SnakePuller::prepare_pull(SnakeBlock* cur) {
    SnakeBlock *prev;
    // The previous snake block is the adjacent one which was pushed.
    // There is at least one such block, and maybe two (but that's fine).
    for (auto& link : cur->links_) {
        if (static_cast<SnakeBlock*>(link)->distance_ == 0) {
            prev = static_cast<SnakeBlock*>(link);
        }
    }
    int d = 1;
    while (true) {
        // If we reach the end of the snake, we can pull it
        if (cur->links_.size() == 1) {
            check_.insert(cur);
            pull(cur);
            break;
        }
        // Progress down the snake
        for (auto link : cur->links_) {
            if (link != prev) {
                cur->distance_ = d++;
                prev = cur;
                cur = static_cast<SnakeBlock*>(link);
                break;
            }
        }
        // If we reach a block with an initialized but shorter distance, we're done
        if (cur->distance_ >= 0 && d >= cur->distance_) {
            // The chain was so short that it didn't break (it was all pushed)!
            if (cur->distance_ <= 1) {
                break;
            }
            // The chain was odd length; split the middle block!
            else if (d == cur->distance_) {
                Point pos = cur->pos();
                room_map_->take(cur, delta_frame_);

                auto a_unique = std::make_unique<SnakeBlock>(pos.x, pos.y, cur->color_, false, 1);
                auto a = a_unique.get();
                room_map_->put(std::move(a_unique), delta_frame_);
                a->target_ = prev;
                a->add_link(prev, delta_frame_);
                pull(a);

                auto b_unique = std::make_unique<SnakeBlock>(pos.x, pos.y, cur->color_, false, 1);
                auto b = b_unique.get();
                room_map_->put(std::move(b_unique), delta_frame_);
                b->target_ = cur->target_;
                b->add_link(cur->target_, delta_frame_);
                pull(b);

                // This snake won't get its target reset otherwise
                // This causes problems post "resurrection" from undo!
                cur->reset_target();
            }
            // The chain was even length; cut!
            else {
                cur->remove_link(prev, delta_frame_);
                pull(cur);
                pull(prev);
            }
            break;
        }
        cur->target_ = prev;
    }
}

void SnakePuller::pull(SnakeBlock* cur) {
    floor_check_.insert(cur->pos_);
    if (cur->ends_ == 2) {
        cur->collect_unlinked_neighbors(room_map_, check_);
    }
    SnakeBlock* next = cur->target_;
    Point cur_pos, next_pos;
    while (next) {
        next_pos = next->pos();
        // When we are sufficiently close to a push, it's possible that the thing
        // we're aiming for has already moved!  In this case, be sure to aim at
        // its previous position instead.
        if (cur->distance() <= 2) {
            cur_pos = cur->pos();
            if (abs(next_pos.x - cur_pos.x) + abs(next_pos.y - cur_pos.y) != 1) {
                next_pos = Point{next_pos.x - dir_.x, next_pos.y - dir_.y};
            }
        }
        cur->set_pos_auto(next_pos, room_map_, delta_frame_);
        cur->reset_target();
        cur = next;
        next = cur->target_;
    }
    cur->reset_target();
}

void SnakeBlock::post_move_reset() {
    reset_target();
}
*/
