#include "room.h"

#include "roommap.h"
#include "camera.h"

#include "graphicsmanager.h"
#include "gameobject.h"
#include "block.h"
#include "snakeblock.h"
#include "door.h"
#include "switch.h"
#include "mapfile.h"

Room::Room(std::string name): name_ {name},
map_ {}, camera_ {}, signalers_ {} {}

Room::Room(std::string name, int w, int h): name_ {name},
map_ {std::make_unique<RoomMap>(w, h)},
camera_ {std::make_unique<Camera>(w, h)},
signalers_ {} {}

Room::~Room() = default;

std::string const Room::name() {
    return name_;
}

void Room::initialize(int w, int h) {
    map_ = std::make_unique<RoomMap>(w, h);
    camera_ = std::make_unique<Camera>(w, h);
}

void Room::set_cam_pos(Point3 pos) {
    camera_->set_current_pos(pos);
}

void Room::set_cam_target(Point3 pos) {
    camera_->set_target(pos);
}

bool Room::valid(Point pos) {
    return (map_ && map_->valid(pos));
}

RoomMap* Room::room_map() {
    return map_.get();
}

void Room::draw(GraphicsManager* gfx, Point3 cam_pos, bool ortho) {
    glm::mat4 model, view, projection;

    if (ortho) {
        camera_->set_current_pos(cam_pos);
        view = glm::lookAt(glm::vec3(cam_pos.x, 2.0f, cam_pos.y),
                           glm::vec3(cam_pos.x, 0.0f, cam_pos.y),
                           glm::vec3(0.0f, 0.0f, -1.0f));
        projection = glm::ortho(-ORTHO_WIDTH/2.0f, ORTHO_WIDTH/2.0f, -ORTHO_HEIGHT/2.0f, ORTHO_HEIGHT/2.0f, 0.0f, 3.0f);
    } else {
        camera_->set_target(cam_pos);
        camera_->update();

        float cam_radius = camera_->get_radius();
        FPoint3 target_pos = camera_->get_pos();

        // NOTE: These belong in the camera class later
        float cam_incline = 0.4;
        float cam_rotation = 0.0;

        float cam_x = sin(cam_incline) * sin(cam_rotation) * cam_radius;
        float cam_y = cos(cam_incline) * cam_radius;
        float cam_z = sin(cam_incline) * cos(cam_rotation) * cam_radius;

        view = glm::lookAt(glm::vec3(cam_x + target_pos.x, cam_y, cam_z + target_pos.y),
                           glm::vec3(target_pos.x, 0.0f, target_pos.y),
                           glm::vec3(0.0f, 1.0f, 0.0f));
        projection = glm::perspective(glm::radians(60.0f), (float)SCREEN_WIDTH/(float)SCREEN_HEIGHT, 0.1f, 100.0f);
    }
    //view = glm::translate(view, glm::vec3(0.5, 0.0, 0.5));
    gfx->set_view(view);
    gfx->set_projection(projection);

    map_->draw(gfx);

    // Draw the floor
    model = glm::translate(glm::mat4(), glm::vec3(-0.5, -0.1, -0.5));
    model = glm::scale(model, glm::vec3(map_->width(), 0.1, map_->height()));
    model = glm::translate(model, glm::vec3(0.5, -0.1, 0.5));
    gfx->set_model(model);
    gfx->set_color(COLORS[YELLOW]);
    gfx->draw_cube();
}

void Room::write_to_file(MapFileO& file, Point3 start_pos) {
    file << MapCode::Dimensions;
    file << map_->width();
    file << map_->height();

    file << MapCode::DefaultPos;
    file << start_pos;

    map_->serialize(file);

    camera_->serialize(file);

    for (auto& signaler : signalers_) {
        signaler->serialize(file);
    }

    file << static_cast<unsigned char>(MapCode::End);
}

void Room::load_from_file(MapFileI& file, Point3* start_pos) {
    unsigned char b[8];
    bool reading_file = true;
    while (reading_file) {
        file.read(b, 1);
        switch (static_cast<MapCode>(b[0])) {
        case MapCode::Dimensions:
            file.read(b, 2);
            initialize(b[0], b[1]);
            break;
        case MapCode::DefaultPos:
            file.read(b, 3);
            if (start_pos) {
                *start_pos = {b[0], b[1], b[2]};
            }
            break;
        case MapCode::Objects:
            read_objects(file);
            break;
        case MapCode::CameraRect:
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

#define CASE_OBJCODE(CLASS)\
case ObjCode::CLASS:\
    map_->put_quiet(std::unique_ptr<GameObject>(CLASS::deserialize(file)));\
    break;

void Room::read_objects(MapFileI& file) {
    unsigned char b[1];
    while (true) {
        file.read(b, 1);
        ObjCode code = static_cast<ObjCode>(b[0]);
        switch (code) {
        CASE_OBJCODE(Wall)
        CASE_OBJCODE(NonStickBlock)
        CASE_OBJCODE(WeakBlock)
        CASE_OBJCODE(StickyBlock)
        CASE_OBJCODE(SnakeBlock)
        CASE_OBJCODE(Door)
        // NOTE: this is a temporary fix to deal with the player for now
        case ObjCode::Player:
            break;
        //CASE_OBJCODE(Player)
        CASE_OBJCODE(PressSwitch)
        CASE_OBJCODE(Gate)
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
    /*while (true) {
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
    }*/
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
    unsigned char b[7];
    file.read(b, 7);
    auto door = static_cast<Door*>(map_->view({b[0],b[1],b[2]}));
    char name[256];
    file.read_str(name, b[6]);
    door->set_dest({b[3],b[4],b[5]}, std::string(name, b[6]));
}

void Room::read_signaler(MapFileI& file) {
    unsigned char b[4];
    file.read(b, 4);
    auto signaler = std::make_unique<Signaler>(b[0], b[1] & 1, b[1] & 2);
    int switches = b[2];
    int switchables = b[3];
    for (int i = 0; i < switches; ++i) {
        signaler->push_switch(static_cast<Switch*>(map_->view(file.read_point3())));
    }
    for (int i = 0; i < switchables; ++i) {
        signaler->push_switchable(static_cast<Switchable*>(map_->view(file.read_point3())));
    }
    push_signaler(std::move(signaler));
}

void Room::push_signaler(std::unique_ptr<Signaler> signaler) {
    signalers_.push_back(std::move(signaler));
}
