#include "snakeblock.h"
#include "graphicsmanager.h"
#include "mapfile.h"
#include "component.h"
#include "delta.h"
#include "roommap.h"

#include <algorithm>

SnakeBlock::SnakeBlock(Point3 pos, ColorCycle color, bool is_car, unsigned char ends):
Block(pos, color, is_car), links_ {}, target_ {}, distance_ {0}, ends_ {ends}  {}

SnakeBlock::~SnakeBlock() {}

ObjCode SnakeBlock::obj_code() {
    return ObjCode::SnakeBlock;
}

void SnakeBlock::draw(GraphicsManager* gfx) {
    Point3 p = pos();
    glm::mat4 model = glm::translate(glm::mat4(), glm::vec3(p.x, p.z, p.y));
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
    for (auto link : links_) {
        Point3 q = link->pos_;
        Point d {q.x - p.x, q.y - p.y};
        gfx->set_color(COLORS[BLACK]);
        model = glm::translate(glm::mat4(), glm::vec3(0.2f*d.x, 0.5f, 0.2f*d.y));
        model = glm::translate(model, glm::vec3(p.x, p.z, p.y));
        model = glm::scale(model, glm::vec3(0.1f + 0.2f*abs(d.x), 0.2f, 0.1f + 0.2f*abs(d.y)));
        gfx->set_model(model);
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
    std::vector<SnakeBlock*> to_check {this};
    while (!to_check.empty()) {
        SnakeBlock* cur = to_check.back();
        to_check.pop_back();
        unique_comp->add_block(cur);
        for (auto link : cur->links_) {
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
            static_cast<SnakeComponent*>(comp_)->set_pushed();
        }
    }
}

void SnakeBlock::get_weak_links(RoomMap* room_map, std::vector<Block*>& links) {
    for (auto link : links_) {
        links.push_back(link);
    }
}

bool SnakeBlock::in_links(SnakeBlock* sb) {
    return std::find(links_.begin(), links_.end(), sb) != links_.end();
}

void SnakeBlock::add_link(SnakeBlock* sb, DeltaFrame* delta_frame) {
    links_.push_back(sb);
    sb->links_.push_back(this);
    if (delta_frame) {
        delta_frame->push(std::make_unique<AddLinkDelta>(this, sb));
    }
}

void SnakeBlock::remove_link(SnakeBlock* sb, DeltaFrame* delta_frame) {
    links_.erase(std::find(links_.begin(), links_.end(), sb));
    sb->links_.erase(std::find(sb->links_.begin(), sb->links_.end(), this));
    if (delta_frame) {
        delta_frame->push(std::make_unique<RemoveLinkDelta>(this, sb));
    }
}

void SnakeBlock::check_add_local_links(RoomMap* room_map, DeltaFrame* delta_frame) {
    if (!available() || confused(room_map)) {
        return;
    }
    for (auto& d : H_DIRECTIONS) {
        auto snake = dynamic_cast<SnakeBlock*>(room_map->view(shifted_pos(d)));
        if (snake && color() == snake->color() && snake->available() && !in_links(snake) && !snake->confused(room_map)) {
            add_link(snake, delta_frame);
        }
    }
}

void SnakeBlock::check_remove_local_links(DeltaFrame* delta_frame) {
    auto links_copy = links_;
    for (auto link : links_copy) {
        Point3 q = link->pos_;
        if (abs(pos_.x - q.x) + abs(pos_.y - q.y) != 1) {
            remove_link(link, delta_frame);
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
        if (snake && color() == snake->color() && (snake->available() || in_links(snake))) {
            ++available_count;
        }
    }
    return available_count > ends_;
}

/*
void SnakeBlock::collect_unlinked_neighbors(RoomMap* room_map, std::unordered_set<SnakeBlock*>& check_snakes) {
    for (auto& d : H_DIRECTIONS) {
        auto snake = dynamic_cast<SnakeBlock*>(room_map->view(shifted_pos(d)));
        if (snake && snake->available()) {
            check_snakes.insert(snake);
        }
    }
}
*/

void SnakeBlock::reset_target() {
    target_ = nullptr;
    distance_ = 0;
}

void SnakeBlock::cleanup() {
    reset_target();
    for (SnakeBlock* link : links_) {
        link->links_.erase(std::find(link->links_.begin(), link->links_.end(), this));
    }
}

void SnakeBlock::reinit() {
    for (SnakeBlock* link : links_) {
        link->links_.push_back(this);
    }
}


SnakePuller::SnakePuller(RoomMap* room_map, DeltaFrame* delta_frame, Point3 dir,
                         std::vector<SnakeBlock*>& check_snakes,
                         std::vector<GameObject*>& below_release):
room_map_ {room_map}, delta_frame_ {delta_frame}, check_snakes_ {check_snakes}, below_release_ {below_release}, dir_ {dir} {}

SnakePuller::~SnakePuller() {}

void SnakePuller::prepare_pull(SnakeBlock* cur) {
    SnakeBlock* prev;
    // The previous snake block is the adjacent one which was pushed.
    // There is at least one such block, and maybe two (but that's fine).
    for (SnakeBlock* link : cur->links_) {
        auto comp = static_cast<SnakeComponent*>(link->s_comp());
        if (comp && comp->pushed()) {
            prev = link;
            break;
        }
    }
    unsigned int d = 1;
    while (true) {
        // If we reach the end of the snake, we can pull it
        if (cur->links_.size() == 1) {
            check_snakes_.push_back(cur);
            GameObject* below = room_map_->view(cur->shifted_pos({0,0,-1}));
            if (below) {
                below_release_.push_back(below);
            }
            pull(cur);
            break;
        }
        // Progress down the snake
        for (SnakeBlock* link : cur->links_) {
            if (link != prev) {
                cur->distance_ = d++;
                prev = cur;
                cur = link;
                break;
            }
        }
        // If we reach another block with a component initialized, nothing gets pulled (yet)
        if (cur->comp_) {
            return;
        }
        // If we reach a block with an initialized but shorter distance, we're done
        if (cur->distance_ > 0 &&  cur->distance_ <= d) {
            // The chain was odd length; split the middle block!
            if (d == cur->distance_) {
                std::vector<SnakeBlock*> links = cur->links_;
                GameObject* below = room_map_->view(cur->shifted_pos({0,0,-1}));
                if (below) {
                    below_release_.push_back(below);
                }
                room_map_->take(cur, delta_frame_);
                for (SnakeBlock* link : links) {
                    Point3 new_pos {link->pos_};
                    if (d == 2) {
                        new_pos -= dir_;
                    }
                    auto split = std::make_unique<SnakeBlock>(new_pos, cur->color_, cur->car_, 1);
                    split->add_link(link, delta_frame_);
                    pull(link);
                    room_map_->put(std::move(split), delta_frame_);
                }
            }
            // The chain was even length; cut!
            else {
                GameObject* below = room_map_->view(cur->shifted_pos({0,0,-1}));
                if (below) {
                    below_release_.push_back(below);
                }
                below = room_map_->view(prev->shifted_pos({0,0,-1}));
                if (below) {
                    below_release_.push_back(below);
                }
                check_snakes_.push_back(cur);
                check_snakes_.push_back(prev);
                cur->remove_link(prev, delta_frame_);
                pull(cur);
                pull(prev);
            }
            return;
        }
        cur->target_ = prev;
    }
}

void SnakePuller::pull(SnakeBlock* cur) {
    if (cur->ends_ == 2) {
        //cur->collect_unlinked_neighbors(room_map_, check_);
    }
    SnakeBlock* next = cur->target_;
    Point3 cur_pos, next_pos;
    std::vector<std::pair<SnakeBlock*, Point3>> to_move {};
    while (next) {
        next_pos = next->pos();
        // When we are sufficiently close to a push, it's possible that the thing
        // we're aiming for has already moved!  In this case, be sure to aim at
        // its previous position instead.
        if (cur->distance_ <= 2) {
            cur_pos = cur->pos();
            if (abs(next_pos.x - cur_pos.x) + abs(next_pos.y - cur_pos.y) != 1) {
                next_pos -= dir_;
            }
        }
        to_move.push_back(std::make_pair(cur, next_pos));
        cur->reset_target();
        cur = next;
        next = cur->target_;
    }
    cur->reset_target();
    for (auto it = to_move.rbegin(); it != to_move.rend(); ++it) {
        std::tie(cur, next_pos) = *it;
        auto cur_unique = room_map_->take_quiet(cur);
        delta_frame_->push(std::make_unique<SingleMoveDelta>(cur, cur->pos_, room_map_));
        cur->set_pos(next_pos);
        room_map_->put_quiet(std::move(cur_unique));
    }
}
