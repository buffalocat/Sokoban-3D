#include "pushblock.h"

#include "objectmodifier.h"
#include "roommap.h"
#include "mapfile.h"
#include "graphicsmanager.h"

PushBlock::PushBlock(Point3 pos, int color, bool pushable, bool gravitable, Sticky sticky):
GameObject(pos, color, pushable, gravitable), sticky_ {sticky} {}

PushBlock::~PushBlock() {}

ObjCode PushBlock::obj_code() {
    return ObjCode::PushBlock;
}

void PushBlock::serialize(MapFileO& file) {
    file << color_ << pushable_ << gravitable_ << sticky_;
}

std::unique_ptr<GameObject> PushBlock::deserialize(MapFileI& file) {
    Point3 pos {file.read_point3()};
    unsigned char b[4];
    file.read(b, 4);
    return std::make_unique<PushBlock>(pos, b[0], b[1], b[2], static_cast<Sticky>(b[3]));
}

void PushBlock::collect_sticky_links(RoomMap* room_map, Sticky sticky_level, std::vector<GameObject*>& links) {
    Sticky sticky_condition = sticky_ & sticky_level;
    if (sticky_condition != Sticky::None) {
        for (Point3 d : DIRECTIONS) {
            PushBlock* adj = dynamic_cast<PushBlock*>(room_map->view(pos_ + d));
            if (adj && adj->color_ == color_ && ((adj->sticky_ & sticky_condition) != Sticky::None)) {
                links.push_back(adj);
            }
        }
    }
}

Sticky PushBlock::sticky() {
    return sticky_;
}


void PushBlock::draw(GraphicsManager* gfx) {
    FPoint3 p {real_pos()};
    glm::mat4 model = glm::translate(glm::mat4(), glm::vec3(p.x, p.z, p.y));
    gfx->set_model(model);
    gfx->set_color(COLORS[color_]);
    switch (sticky_) {
    case Sticky::None:
        gfx->set_tex(Texture::Edges);
        break;
    case Sticky::Weak:
        gfx->set_tex(Texture::Corners);
        break;
    case Sticky::Strong:
        gfx->set_tex(Texture::Blank);
        break;
    }
    gfx->draw_cube();
    // TODO: once walls are a thing, make sure to turn this back on!
    // Also, give walls a unique texture, maybe?
    //draw_force_indicators(gfx, model);
    if (modifier_) {
        modifier()->draw(gfx, p);
    }
}
