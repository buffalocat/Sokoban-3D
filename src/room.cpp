#include "room.h"

#include "roommap.h"
#include "camera.h"

#include "graphicsmanager.h"
#include "gameobject.h"

#include "pushblock.h"
#include "snakeblock.h"
#include "door.h"
#include "switch.h"
#include "switchable.h"
#include "signaler.h"
#include "mapfile.h"

Room::Room(std::string name): name_ {name},
map_ {}, camera_ {}, offset_pos_ {0,0} {}

Room::~Room() {}

std::string const Room::name() {
    return name_;
}

void Room::initialize(GameObjectArray& objs, int w, int h, int d) {
    map_ = std::make_unique<RoomMap>(objs, w, h, d);
    camera_ = std::make_unique<Camera>(w, h);
}

void Room::set_cam_pos(Point3 pos) {
    camera_->set_current_pos(pos);
}

bool Room::valid(Point3 pos) {
    return (map_ && map_->valid(pos));
}

RoomMap* Room::room_map() {
    return map_.get();
}

void Room::draw(GraphicsManager* gfx, Point3 cam_pos, bool ortho, bool one_layer) {
    update_view(gfx, cam_pos, cam_pos, ortho);
    if (one_layer) {
        map_->draw_layer(gfx, cam_pos.z);
    } else {
        map_->draw(gfx, camera_->get_rotation());
    }
}

void Room::draw(GraphicsManager* gfx, GameObject* target, bool ortho, bool one_layer) {
    update_view(gfx, target->pos_, target->real_pos(), ortho);
    if (one_layer) {
        map_->draw_layer(gfx, target->pos_.z);
    } else {
        map_->draw(gfx, camera_->get_rotation());
    }
}

void Room::update_view(GraphicsManager* gfx, Point3 vpos, FPoint3 rpos, bool ortho) {
    glm::mat4 model, view, projection;
    if (ortho) {
        camera_->set_current_pos(rpos);
        view = glm::lookAt(glm::vec3(rpos.x, rpos.z, rpos.y),
                           glm::vec3(rpos.x, rpos.z - 1.0f, rpos.y),
                           glm::vec3(0.0f, 0.0f, -1.0f));
        projection = glm::ortho(-ORTHO_WIDTH/2.0f, ORTHO_WIDTH/2.0f, -ORTHO_HEIGHT/2.0f, ORTHO_HEIGHT/2.0f, -2.5f, 2.5f);
    } else {
        camera_->set_target(vpos, rpos);
        camera_->update();

        float cam_radius = camera_->get_radius();
        FPoint3 target_pos = camera_->get_pos();

        float cam_tilt = camera_->get_tilt();
        float cam_rotation = camera_->get_rotation();
        float cam_x = sin(cam_tilt) * sin(cam_rotation) * cam_radius;
        float cam_y = cos(cam_tilt) * cam_radius;
        float cam_z = sin(cam_tilt) * cos(cam_rotation) * cam_radius;

        view = glm::lookAt(glm::vec3(cam_x + target_pos.x, cam_y, cam_z + target_pos.y),
                           glm::vec3(target_pos.x, 0.0f, target_pos.y),
                           glm::vec3(0.0f, 1.0f, 0.0f));
        projection = glm::perspective(glm::radians(60.0f), (float)SCREEN_WIDTH/(float)SCREEN_HEIGHT, 0.1f, 100.0f);
    }
    gfx->set_view(view);
    gfx->set_projection(projection);
}

void Room::write_to_file(MapFileO& file, Point3 start_pos) {
    file << MapCode::Dimensions;
    file << map_->width_;
    file << map_->height_;

    file << MapCode::DefaultPos;
    file << start_pos;

    map_->serialize(file);

    camera_->serialize(file);

    file << MapCode::End;
}

void Room::load_from_file(GameObjectArray& objs, MapFileI& file, Point3* start_pos) {
    unsigned char b[8];
    bool reading_file = true;
    while (reading_file) {
        file.read(b, 1);
        switch (static_cast<MapCode>(b[0])) {
        case MapCode::Dimensions:
            file.read(b, 3);
            initialize(objs, b[0], b[1], b[2]);
            break;
        case MapCode::FullLayer:
            map_->push_full();
            break;
        case MapCode::SparseLayer:
            map_->push_sparse();
            break;
        case MapCode::DefaultPos:
            file.read(b, 3);
            if (start_pos) {
                *start_pos = {b[0], b[1], b[2]};
            }
            break;
        case MapCode::OffsetPos:
            file >> offset_pos_;
            break;
        case MapCode::Objects:
            read_objects(file);
            break;
        case MapCode::CameraRects:
            read_camera_rects(file);
            break;
        case MapCode::SnakeLink:
            read_snake_link(file);
            break;
        case MapCode::DoorDest :
            read_door_dest(file);
            break;
        case MapCode::Signaler:
            read_signaler(file);
            break;
        case MapCode::Walls:
            read_walls(file);
            break;
        case MapCode::PlayerData:
            read_player_data(file);
            break;
        case MapCode::End:
            reading_file = false;
            break;
        default :
            std::cout << "unknown state code! " << b[0] << std::endl;
            //throw std::runtime_error("Unknown State code encountered in .map file (it's probably corrupt/an old version)");
            break;
        }
    }
}

// TODO: fix (de)serialization in general!!!!!

#define CASE_OBJCODE(CLASS)\
case ObjCode::CLASS:\
    map_->create(std::move(CLASS::deserialize(file)));\
    break;

void Room::read_objects(MapFileI& file) {
    unsigned char b;
    while (true) {
        file.read(&b, 1);
        switch (static_cast<ObjCode>(b)) {
        CASE_OBJCODE(PushBlock)
        CASE_OBJCODE(SnakeBlock)
        // Walls should never be encountered here, so ignore them
        case ObjCode::Wall:
            break;
        // NOTE: this is a temporary fix to deal with the player for now
        // The Player's position SHOULD be saved in real save files
        case ObjCode::Player:
            break;
        case ObjCode::NONE:
            return;
        default :
            throw std::runtime_error("Unknown Object code encountered in .map file (it's probably corrupt/an old version)");
            break;
        }
    }
}

#undef CASE_OBJCODE

#define CASE_CAMCODE(CLASS)\
case CameraCode::CLASS:\
    camera_->push_context(std::unique_ptr<CameraContext>(CLASS ## CameraContext::deserialize(file)));\
    break;

void Room::read_camera_rects(MapFileI& file) {
    unsigned char b[1];
    while (true) {
        file.read(b, 1);
        CameraCode code = static_cast<CameraCode>(b[0]);
        switch (code) {
        CASE_CAMCODE(Free)
        CASE_CAMCODE(Fixed)
        CASE_CAMCODE(Clamped)
        CASE_CAMCODE(Null)
        case CameraCode::NONE:
            return;
        default :
            throw std::runtime_error("Unknown Camera code encountered in .map file (it's probably corrupt/an old version)");
            return;
        }
    }
}

#undef CASE_CAMCODE

void Room::read_snake_link(MapFileI& file) {
    unsigned char b[4];
    file.read(b, 4);
    SnakeBlock* sb = static_cast<SnakeBlock*>(map_->view({b[0], b[1], b[2]}));
    // Linked right
    if (b[3] & 1) {
        sb->add_link(static_cast<SnakeBlock*>(map_->view({b[0]+1, b[1], b[2]})), nullptr);
    }
    // Linked down
    if (b[3] & 2) {
        sb->add_link(static_cast<SnakeBlock*>(map_->view({b[0], b[1]+1, b[2]})), nullptr);
    }
}

void Room::read_door_dest(MapFileI& file) {
    Point3 pos {file.read_point3()};
    Point3 exit_pos {file.read_point3()};
    auto door = static_cast<Door*>(map_->view(pos)->modifier());
    door->set_dest(exit_pos, file.read_str());
}

void Room::read_signaler(MapFileI& file) {
    unsigned char b[5];
    file.read(b, 5);
    auto signaler = std::make_unique<Signaler>(b[0], b[1], b[2]);
    for (int i = 0; i < b[3]; ++i) {
        signaler->push_switch(static_cast<Switch*>(map_->view(file.read_point3())->modifier()));
    }
    for (int i = 0; i < b[4]; ++i) {
        signaler->push_switchable(static_cast<Switchable*>(map_->view(file.read_point3())->modifier()));
    }
    map_->push_signaler(std::move(signaler));
}

void Room::read_walls(MapFileI& file) {

}

void Room::read_player_data(MapFileI& file) {

}
