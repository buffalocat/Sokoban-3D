#include "block.h"
#include "shader.h"
#include "delta.h"
#include "roommap.h"
#include "moveprocessor.h"

BlockSet Block::EMPTY_BLOCK_SET {};

// Push is the "default" BlockType
Block::Block(int x, int y): GameObject(x, y), color_ {0}, car_ {false}, links_ {} {}

Block::Block(int x, int y, unsigned char color, bool is_car): GameObject(x, y), color_ {color}, car_ {is_car}, links_ {} {}

Block::~Block() {}

Layer Block::layer() {
    return Layer::Solid;
}

bool Block::is_car() {
    return car_;
}

void Block::set_car(bool is_car) {
    car_ = is_car;
}

void Block::draw(Shader* shader) {
    Point p = pos();
    glm::mat4 model;
    if (car_) {
        model = glm::translate(glm::mat4(), glm::vec3(p.x, 1.0f, p.y));
        model = glm::scale(model, glm::vec3(0.7f, 0.1f, 0.7f));
        shader->setMat4("model", model);
        shader->setVec4("color", COLORS[LIGHT_GREY]);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, nullptr);
    }
    // Debugging mode!! Maybe this will be toggle-able later?
    shader->setVec4("color", COLORS[BLACK]);
    for (auto link : links_) {
        Point q = link->pos();
        Point d {q.x - p.x, q.y - p.y};
        model = glm::translate(glm::mat4(), glm::vec3(0.2f*d.x, 0.5f, 0.2f*d.y));
        model = glm::translate(model, glm::vec3(p.x, 0.5f, p.y));
        model = glm::scale(model, glm::vec3(0.1f + 0.2f*abs(d.x), 0.2f, 0.1f + 0.2f*abs(d.y)));
        shader->setMat4("model", model);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, nullptr);
    }
}

unsigned char Block::color() {
    return color_;
}

void Block::add_link(Block* link, DeltaFrame* delta_frame) {
    if (links_.count(link))
        return;
    links_.insert(link);
    link->links_.insert(this);
    if (delta_frame)
        delta_frame->push(std::make_unique<AddLinkDelta>(this, link));
}

void Block::remove_link(Block* link, DeltaFrame* delta_frame) {
    if (!links_.count(link))
        return;
    links_.erase(link);
    link->links_.erase(this);
    if (delta_frame)
        delta_frame->push(std::make_unique<RemoveLinkDelta>(this, link));
}

// This only gets called during room change, with a non-trivial DeltaFrame
void Block::remove_all_links(DeltaFrame* delta_frame) {
    for (auto link : links_) {
        links_.erase(link);
        link->links_.erase(this);
        delta_frame->push(std::make_unique<RemoveLinkDelta>(this, link));
    }
}

void Block::cleanup(DeltaFrame* delta_frame) {
    // When a Block is destroyed, any links to it should be forgotten
    for (auto link : links_) {
        link->links_.erase(this);
    }
}

void Block::post_move_reset() {}

void Block::reinit() {
    // When a Block is un-destroyed, any links it had should be reconnected
    for (auto link : links_) {
        link->links_.insert(this);
    }
}

void Block::check_remove_local_links(RoomMap* room_map, DeltaFrame* delta_frame) {
    Point p, q;
    p = pos();
    for (auto& link : links_) {
        q = link->pos();
        if (abs(p.x - q.x) + abs(p.y - q.y) >= 2) {
            remove_link(link, delta_frame);
        }
    }
}

PushBlock::PushBlock(int x, int y): Block(x, y, 0, false), sticky_ {StickyLevel::None} {}
PushBlock::PushBlock(int x, int y, unsigned char color, bool is_car, StickyLevel sticky): Block(x, y, color, is_car), sticky_ {sticky} {}

PushBlock::~PushBlock() {}

ObjCode PushBlock::obj_code() {
    return ObjCode::PushBlock;
}

void PushBlock::set_sticky(StickyLevel sticky) {
    sticky_ = sticky;
}

void PushBlock::draw(Shader* shader) {
    Point p = pos();
    glm::mat4 model = glm::translate(glm::mat4(), glm::vec3(p.x, 0.5f, p.y));
    shader->setMat4("model", model);
    shader->setVec4("color", COLORS[color_]);
    if (sticky_ == StickyLevel::None) {
        shader->setVec2("TexOffset", glm::vec2(2,0));
    } else if (sticky_ == StickyLevel::Weak) {
        shader->setVec2("TexOffset", glm::vec2(1,0));
    }
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, nullptr);
    shader->setVec2("TexOffset", glm::vec2(0,0));
    Block::draw(shader);
}

StickyLevel PushBlock::sticky() {
    return sticky_;
}

const BlockSet& PushBlock::get_strong_links() {
    if (sticky_ == StickyLevel::Strong) {
        return links_;
    } else {
        return EMPTY_BLOCK_SET;
    }
}

const BlockSet& PushBlock::get_weak_links() {
    if (sticky_ == StickyLevel::Weak) {
        return links_;
    } else {
        return EMPTY_BLOCK_SET;
    }
}

void PushBlock::serialize(std::ofstream& file) {
    file << color_;
    unsigned char ser = static_cast<unsigned char>(sticky_); // StickyLevel stored in first two bits
    if (car_) {
        ser |= 1 << 7; // car stored in 8th bit
    }
    file << ser;
}

GameObject* PushBlock::deserialize(unsigned char* b) {
    bool is_car = (b[3] >> 7) == 1;
    StickyLevel sticky = static_cast<StickyLevel>(b[3] & 3);
    return new PushBlock(b[0], b[1], b[2], is_car, sticky);
}

bool PushBlock::relation_check() {
    return false;
}

void PushBlock::relation_serialize(std::ofstream& file) {}

bool PushBlock::push_recheck(MoveProcessor* mp) {
    return false;
}

void PushBlock::check_add_local_links(RoomMap* room_map, DeltaFrame* delta_frame) {
    if (sticky_ == StickyLevel::None)
        return;
    for (auto& d : DIRECTIONS) {
        auto block = dynamic_cast<PushBlock*>(room_map->view(shifted_pos(d), Layer::Solid));
        if (block && sticky_ == block->sticky_ && color_ == block->color_) {
            add_link(block, delta_frame);
        }
    }
}

SnakeBlock::SnakeBlock(int x, int y): Block(x, y), ends_ {2}, distance_ {-1}, target_ {nullptr} {}
SnakeBlock::SnakeBlock(int x, int y, unsigned char color, bool is_car, unsigned int ends): Block(x, y, color, is_car), ends_ {(ends == 1 || ends == 2) ? ends : 2}, distance_ {-1}, target_ {nullptr} {}

SnakeBlock::~SnakeBlock() {}

ObjCode SnakeBlock::obj_code() {
    return ObjCode::SnakeBlock;
}

const BlockSet& SnakeBlock::get_strong_links() {
    return EMPTY_BLOCK_SET;
}

// A snake that wasn't pushed pretends it doesn't have any links
const BlockSet& SnakeBlock::get_weak_links() {
    if (distance_ > 0) {
        return EMPTY_BLOCK_SET;
    }
    return links_;
}

unsigned int SnakeBlock::ends() {
    return ends_;
}

int SnakeBlock::distance() {
    return distance_;
}

void SnakeBlock::reset_target() {
    target_ = nullptr;
    distance_ = -1;
}

void SnakeBlock::draw(Shader* shader) {
    Point p = pos();
    glm::mat4 model = glm::translate(glm::mat4(), glm::vec3(p.x, 0.5f, p.y));
    model = glm::scale(model, glm::vec3(0.7071f, 1, 0.7071f));
    model = glm::rotate(model, glm::radians(45.0f), glm::vec3(0, 1, 0));
    shader->setMat4("model", model);
    shader->setVec4("color", COLORS[color_]);
    if (ends_ == 1) {
        shader->setVec2("TexOffset", glm::vec2(2,0));
    }
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, nullptr);
    shader->setVec2("TexOffset", glm::vec2(0,0));
    Block::draw(shader);
}

void SnakeBlock::serialize(std::ofstream& file) {
    file << color_;
    unsigned char ser = ends_ - 1; // ends - 1 in 1st bit
    Point p, q;
    p = pos_;
    for (auto& link : links_) {
        q = link->pos();
        if (p.x != q.x) {
            ser |= 1 << (q.x - p.x + 2); // dx stored in 2nd and 4th bits
        } else {
            ser |= 1 << (q.y - p.y + 3); // dy stored in 3rd and 5th bits
        }
    }
    if (car_) {
        ser |= 1 << 7; // car stored in 8th bit
    }
    file << ser;
}

GameObject* SnakeBlock::deserialize(unsigned char* b) {
    bool is_car = (b[3] >> 7) == 1;
    int ends = (b[3] & 1) + 1;
    return new SnakeBlock(b[0], b[1], b[2], is_car, ends);
}

bool SnakeBlock::relation_check() {
    return true;
}

void SnakeBlock::relation_serialize(std::ofstream& file) {
    unsigned char link_encode = 0;
    for (auto& link : links_) {
        Point q = link->pos();
        // Snake links are always adjacent, and we only bother to
        // record links to the Right or Down
        if (q.x > pos_.x) {
            ++link_encode;
        } else if (q.y > pos_.y) {
            link_encode += 2;
        }
    }
    if (link_encode) {
        file << static_cast<unsigned char>(State::SnakeLink);
        file << static_cast<unsigned char>(pos_.x);
        file << static_cast<unsigned char>(pos_.y);
        file << link_encode;
    }
}

bool SnakeBlock::push_recheck(MoveProcessor* mp) {
    bool recheck = (distance_ == 1);
    target_ = nullptr;
    distance_ = 0;
    mp->insert_touched_snake(this);
    for (auto& link : links_) {
        auto snake = static_cast<SnakeBlock*>(link);
        // A snake has distance 0 if and only if it has been pushed
        // An unpushed snake next to a pushed snake will be taken over
        if (snake->distance_) {
            snake->target_ = nullptr;
            snake->distance_ = 1;
            mp->insert_touched_snake(snake);
        }
    }
    return recheck;
}

void SnakeBlock::check_add_local_links(RoomMap* room_map, DeltaFrame* delta_frame) {
    if (!available() || confused(room_map)) {
        return;
    }
    for (auto& d : DIRECTIONS) {
        auto snake = dynamic_cast<SnakeBlock*>(room_map->view(shifted_pos(d), Layer::Solid));
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
    for (auto& d : DIRECTIONS) {
        auto snake = dynamic_cast<SnakeBlock*>(room_map->view(shifted_pos(d), Layer::Solid));
        if (snake && color_ == snake->color_ && (snake->available() || links_.count(snake))) {
            ++available_count;
        }
    }
    return available_count > ends_;
}

void SnakeBlock::collect_unlinked_neighbors(RoomMap* room_map, std::unordered_set<SnakeBlock*>& check_snakes) {
    for (auto& d : DIRECTIONS) {
        auto snake = dynamic_cast<SnakeBlock*>(room_map->view(shifted_pos(d), Layer::Solid));
        if (snake && snake->available()) {
            check_snakes.insert(snake);
        }
    }
}

SnakePuller::SnakePuller(RoomMap* room_map, DeltaFrame* delta_frame, std::unordered_set<SnakeBlock*>& check, PointSet& floor_check, Point dir):
room_map_ {room_map}, delta_frame_ {delta_frame}, check_ {check}, floor_check_ {floor_check}, dir_ {dir} {}

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
