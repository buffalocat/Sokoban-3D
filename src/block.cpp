#include "block.h"
#include "shader.h"
#include "delta.h"
#include "roommap.h"
#include "moveprocessor.h"

BlockSet Block::EMPTY_BLOCK_SET {};

// Push is the "default" BlockType
Block::Block(int x, int y): GameObject(x, y), car_ {false}, links_ {} {
    wall_ = false;
}

Block::Block(int x, int y, bool car): GameObject(x, y), car_ {car}, links_ {} {
    wall_ = false;
}

Block::~Block() {}

bool Block::car() {
    return car_;
}

void Block::set_car(bool car) {
    car_ = car;
}

void Block::draw(Shader* shader, int height) {
    Point p = pos();
    glm::mat4 model;
    if (car_) {
        model = glm::translate(glm::mat4(), glm::vec3(p.x, 1.0f + height, p.y));
        model = glm::scale(model, glm::vec3(0.5f, 0.2f, 0.5f));
        shader->setMat4("model", model);
        shader->setVec4("color", PINK);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, nullptr);
    }
    // Debugging mode!! Maybe this will be toggle-able later?
    shader->setVec4("color", BLACK);
    for (auto link : links_) {
        Point q = link->pos();
        Point d {q.x - p.x, q.y - p.y};
        model = glm::translate(glm::mat4(), glm::vec3(0.2f*d.x, 0.5f + height, 0.2f*d.y));
        model = glm::translate(model, glm::vec3(p.x, 0.5f, p.y));
        model = glm::scale(model, glm::vec3(0.1f + 0.2f*abs(d.x), 0.1f, 0.1f + 0.2f*abs(d.y)));
        shader->setMat4("model", model);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, nullptr);
    }
}

void Block::shift_pos(Point d, DeltaFrame* delta_frame) {
    if (delta_frame) {
        delta_frame->push(std::make_unique<MotionDelta>(this, pos_));
    }
    pos_.x += d.x;
    pos_.y += d.y;
}

void Block::set_pos(Point p, DeltaFrame* delta_frame) {
    if (delta_frame) {
        delta_frame->push(std::make_unique<MotionDelta>(this, pos_));
    }
    pos_.x = p.x;
    pos_.y = p.y;
}

BlockSet Block::links() {
    return links_;
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

PushBlock::PushBlock(int x, int y): Block(x, y, false), sticky_ {StickyLevel::None} {}
PushBlock::PushBlock(int x, int y, bool car, StickyLevel sticky): Block(x, y, car), sticky_ {sticky} {}

PushBlock::~PushBlock() {}

void PushBlock::set_sticky(StickyLevel sticky) {
    sticky_ = sticky;
}

void PushBlock::draw(Shader* shader, int height) {
    Point p = pos();
    glm::mat4 model = glm::translate(glm::mat4(), glm::vec3(p.x, 0.5f + height, p.y));
    shader->setMat4("model", model);
    if (sticky_ == StickyLevel::None) {
        shader->setVec4("color", GREEN);
    } else if (sticky_ == StickyLevel::Strong) {
        shader->setVec4("color", ORANGE);
    } else /* sticky_ == StickyLevel::Weak */ {
        shader->setVec4("color", RED);
    }
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, nullptr);
    Block::draw(shader, height);
}

StickyLevel PushBlock::sticky() {
    return sticky_;
}

ObjCode PushBlock::obj_code() {
    return ObjCode::PushBlock;
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
    unsigned char ser = static_cast<unsigned char>(sticky_); // StickyLevel stored in first two bits
    if (car_) {
        ser |= 1 << 7; // car stored in 8th bit
    }
    file << ser;
}

bool PushBlock::push_recheck(MoveProcessor* mp) {
    return false;
}

void PushBlock::check_add_local_links(RoomMap* room_map, DeltaFrame* delta_frame) {
    if (sticky_ == StickyLevel::None)
        return;
    for (auto& d : DIRECTIONS) {
        auto block = dynamic_cast<PushBlock*>(room_map->view(shifted_pos(d), Layer::Solid));
        if (block && sticky_ == block->sticky_) {
            add_link(block, delta_frame);
        }
    }
}

SnakeBlock::SnakeBlock(int x, int y): Block(x, y), ends_ {2}, distance_ {-1}, target_ {nullptr} {}
SnakeBlock::SnakeBlock(int x, int y, bool car, unsigned int ends): Block(x, y, car), ends_ {(ends == 1 || ends == 2) ? ends : 2}, distance_ {-1}, target_ {nullptr} {}

SnakeBlock::~SnakeBlock() {}

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

void SnakeBlock::draw(Shader* shader, int height) {
    Point p = pos();
    glm::mat4 model = glm::translate(glm::mat4(), glm::vec3(p.x, 0.5f + height, p.y));
    model = glm::scale(model, glm::vec3(0.7071f, 1, 0.7071f));
    model = glm::rotate(model, glm::radians(45.0f), glm::vec3(0, 1, 0));
    shader->setMat4("model", model);
    if (ends_ == 1) {
        shader->setVec4("color", PURPLE);
    } else {
        shader->setVec4("color", DARK_PURPLE);
    }
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, nullptr);
    Block::draw(shader, height);
}

ObjCode SnakeBlock::obj_code() {
    return ObjCode::SnakeBlock;
}

void SnakeBlock::serialize(std::ofstream& file) {
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
        if (snake && snake->available() && !snake->confused(room_map)) {
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
        if (snake && (snake->available() || links_.count(snake))) {
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

// NOTE: pull and pull_aux are fairly complicated and share quite a lot of information
// Maybe refactor to a method object later?
void SnakeBlock::pull(RoomMap* room_map, DeltaFrame* delta_frame, std::unordered_set<SnakeBlock*>& check_snakes, Point dir) {
    SnakeBlock *prev, *cur;
    cur = this;
    // The previous snake block is the adjacent one which was pushed.
    // There is at least one such block, and maybe two (but that's fine).
    for (auto& link : links_) {
        if (static_cast<SnakeBlock*>(link)->distance_ == 0) {
            prev = static_cast<SnakeBlock*>(link);
        }
    }
    int d = 1;
    while (true) {
        // If we reach the end of the snake, we can pull it
        if (cur->links().size() == 1) {
            check_snakes.insert(cur);
            cur->pull_aux(room_map, delta_frame, check_snakes, dir);
            break;
        }
        // Progress down the snake
        for (auto link : cur->links()) {
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
                room_map->take_id(pos, Layer::Solid, cur, delta_frame);

                auto a_unique = std::make_unique<SnakeBlock>(pos.x, pos.y, false, 1);
                auto a = a_unique.get();
                room_map->put(std::move(a_unique), delta_frame);
                a->target_ = prev;
                a->add_link(prev, delta_frame);
                a->pull_aux(room_map, delta_frame, check_snakes, dir);

                auto b_unique = std::make_unique<SnakeBlock>(pos.x, pos.y, false, 1);
                auto b = b_unique.get();
                room_map->put(std::move(b_unique), delta_frame);
                b->target_ = cur->target_;
                b->add_link(cur->target_, delta_frame);
                b->pull_aux(room_map, delta_frame, check_snakes, dir);

                // This snake won't get its target reset otherwise
                // This causes problems post "resurrection" from undo!
                cur->reset_target();
            }
            // The chain was even length; cut!
            else {
                cur->remove_link(prev, delta_frame);
                cur->pull_aux(room_map, delta_frame, check_snakes, dir);
                prev->pull_aux(room_map, delta_frame, check_snakes, dir);
            }
            break;
        }
        cur->target_ = prev;
    }
}

void SnakeBlock::pull_aux(RoomMap* room_map, DeltaFrame* delta_frame, std::unordered_set<SnakeBlock*>& check_snakes, Point dir) {
    if (ends_ == 2) {
        collect_unlinked_neighbors(room_map, check_snakes);
    }
    SnakeBlock* cur = this;
    SnakeBlock* next = cur->target_;
    Point cur_pos, next_pos;
    while (next) {
        auto obj_unique = room_map->take_quiet_id(cur->pos(), Layer::Solid, cur);
        next_pos = next->pos();
        // When we are sufficiently close to a push, it's possible that the thing
        // we're aiming for has already moved!  In this case, be sure to aim at
        // its previous position instead.
        if (cur->distance() <= 2) {
            cur_pos = cur->pos();
            if (abs(next_pos.x - cur_pos.x) + abs(next_pos.y - cur_pos.y) != 1) {
                next_pos = Point{next_pos.x - dir.x, next_pos.y - dir.y};
            }
        }
        cur->set_pos(next_pos, delta_frame);
        cur->reset_target();
        room_map->put_quiet(std::move(obj_unique));
        cur = next;
        next = cur->target_;
    }
    cur->reset_target();
}

void SnakeBlock::post_move_reset() {
    reset_target();
}
