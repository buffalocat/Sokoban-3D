#include "room.h"

#include "roommap.h"
#include "camera.h"

#include "gameobject.h"
#include "graphicsmanager.h"
#include "block.h"
#include "door.h"
#include "switch.h"

Room::Room(std::string name): name_ {name},
map_ {}, camera_ {}, signalers_ {} {}

Room::Room(std::string name, int w, int h): name_ {name},
map_ {std::make_unique<RoomMap>(w, h)},
camera_ {std::make_unique<Camera>(w, h)},
signalers_ {} {}

std::string const Room::name() {
    return name_;
}

void Room::initialize(int w, int h) {
    map_ = std::make_unique<RoomMap>(w, h);
    camera_ = std::make_unique<Camera>(w, h);
}

void Room::set_cam_pos(Point pos) {
    camera_->set_current_pos(pos);
}

void Room::set_cam_target(Point pos) {
    camera_->set_target(pos);
}

bool Room::valid(Point pos) {
    return (map_ && map_->valid(pos));
}

RoomMap* Room::room_map() {
    return map_.get();
}

void Room::draw(GraphicsManager* gfx, Point cam_pos, bool ortho) {
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
        FPoint target_pos = camera_->get_pos();

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

void Room::write_to_file(std::ofstream& file, Point start_pos) {
    file << static_cast<unsigned char>(MapCode::Dimensions);
    file << static_cast<unsigned char>(map_->width());
    file << static_cast<unsigned char>(map_->height());

    file << static_cast<unsigned char>(MapCode::DefaultPos);
    file << static_cast<unsigned char>(start_pos.x);
    file << static_cast<unsigned char>(start_pos.y);

    map_->serialize(file);

    camera_->serialize(file);

    for (auto& signaler : signalers_) {
        signaler->serialize(file);
    }

    file << static_cast<unsigned char>(MapCode::End);
}

void Room::load_from_file(std::ifstream& file, Point* start_pos) {
    unsigned char b[8];
    bool reading_file = true;
    while (reading_file) {
        file.read((char *)b, 1);
        switch (static_cast<MapCode>(b[0])) {
        case MapCode::Dimensions :
            file.read((char *)b, 2);
            initialize(b[0], b[1]);
            break;
        case MapCode::DefaultPos :
            file.read((char *)b, 2);
            if (start_pos) {
                *start_pos = {b[0], b[1]};
            }
            break;
        case MapCode::Objects :
            read_objects(file);
            break;
        case MapCode::CameraRect :
            read_camera_rects(file);
            break;
        case MapCode::SnakeLink :
            read_snake_link(file);
            break;
        case MapCode::DoorDest :
            read_door_dest(file);
            break;
        case MapCode::Signaler :
            read_signaler(file);
            break;
        case MapCode::End :
            reading_file = false;
            break;
        default :
            std::cout << "unknown state code! " << b[0] << std::endl;
            //throw std::runtime_error("Unknown State code encountered in .map file (it's probably corrupt/an old version)");
            break;
        }
    }
}

const std::unordered_map<ObjCode, unsigned int, ObjCodeHash> BYTES_PER_OBJECT = {
    {ObjCode::NONE, 0},
    {ObjCode::Wall, 2},
    {ObjCode::PushBlock, 4},
    {ObjCode::SnakeBlock, 4},
    {ObjCode::Door, 2},
    {ObjCode::Player, 3},
    {ObjCode::PlayerWall, 2},
    {ObjCode::PressSwitch, 4},
    {ObjCode::Gate, 3},
};

const std::unordered_map<CameraCode, unsigned int, CameraCodeHash> BYTES_PER_CAMERA = {
    {CameraCode::NONE, 0},
    {CameraCode::Free, 9},
    {CameraCode::Fixed, 13},
    {CameraCode::Clamped, 11},
    {CameraCode::Null, 5},
};

#define CASE_OBJCODE(CLASS)\
case ObjCode::CLASS :\
    map_->put_quiet(std::unique_ptr<GameObject>(CLASS::deserialize(b)));\
    break;

void Room::read_objects(std::ifstream& file) {
    unsigned char b[8];
    while (true) {

        file.read(reinterpret_cast<char *>(b), 1);
        ObjCode code = static_cast<ObjCode>(b[0]);
        file.read((char *)b, BYTES_PER_OBJECT.at(code));
        switch (code) {
        CASE_OBJCODE(Wall)
        CASE_OBJCODE(PushBlock)
        CASE_OBJCODE(SnakeBlock)
        CASE_OBJCODE(Door)
        // NOTE: this is a temporary fix to deal with the player for now
        case ObjCode::Player :
            break;
        //CASE_OBJCODE(Player)
        CASE_OBJCODE(PlayerWall)
        CASE_OBJCODE(PressSwitch)
        CASE_OBJCODE(Gate)
        case ObjCode::NONE :
            return;
        default :
            throw std::runtime_error("Unknown Object code encountered in .map file (it's probably corrupt/an old version)");
            break;
        }
    }
}

#undef CASE_OBJCODE

#define CASE_CAMCODE(CLASS)\
case CameraCode::CLASS :\
    camera_->push_context(std::unique_ptr<CameraContext>(CLASS ## CameraContext::deserialize(b)));\
    break;

void Room::read_camera_rects(std::ifstream& file) {
    unsigned char b[16];
    while (true) {
        file.read(reinterpret_cast<char *>(b), 1);
        CameraCode code = static_cast<CameraCode>(b[0]);
        file.read((char *)b, BYTES_PER_CAMERA.at(code));
        switch (code) {
        CASE_CAMCODE(Free)
        CASE_CAMCODE(Fixed)
        CASE_CAMCODE(Clamped)
        CASE_CAMCODE(Null)
        case CameraCode::NONE :
            return;
        default :
            throw std::runtime_error("Unknown Camera code encountered in .map file (it's probably corrupt/an old version)");
            return;
        }
    }
}

#undef CASE_CAMCODE

void Room::read_snake_link(std::ifstream& file) {
    unsigned char b[3];
    file.read((char *)b, 3);
    SnakeBlock* sb = static_cast<SnakeBlock*>(map_->view(Point{b[0], b[1]}, Layer::Solid));
    // Linked right
    if (b[2] & 1) {
        sb->add_link(static_cast<SnakeBlock*>(map_->view(Point{b[0]+1, b[1]}, Layer::Solid)), nullptr);
    }
    // Linked down
    if (b[2] & 2) {
        sb->add_link(static_cast<SnakeBlock*>(map_->view(Point{b[0], b[1]+1}, Layer::Solid)), nullptr);
    }
}

void Room::read_door_dest(std::ifstream& file) {
    unsigned char b[5];
    file.read((char *)b, 5);
    auto door = static_cast<Door*>(map_->view(Point{b[0],b[1]}, ObjCode::Door));
    char name[256];
    file.read(name, b[4]);
    door->set_dest(Point{b[2],b[3]}, std::string(name, b[4]));
}

void Room::read_signaler(std::ifstream& file) {
    unsigned char b[4];
    file.read((char*)b, 4);
    auto signaler = std::make_unique<Signaler>(b[0], b[1] & 1, b[1] & 2);
    int switches = b[2];
    int switchables = b[3];
    for (int i = 0; i < switches; ++i) {
        file.read((char*)b, 3);
        signaler->push_switch(static_cast<Switch*>(map_->view(Point {b[0],b[1]}, (ObjCode)b[2])));
    }
    for (int i = 0; i < switchables; ++i) {
        file.read((char*)b, 3);
        signaler->push_switchable(static_cast<Switchable*>(map_->view(Point {b[0],b[1]}, (ObjCode)b[2])));
    }
    signalers_.push_back(std::move(signaler));
}
