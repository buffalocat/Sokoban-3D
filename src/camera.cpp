#include "camera.h"
#include "roommap.h"
#include <algorithm>

// Because we only work with dyadic rationals,
// the floats that get read in and out should
// never accumulate any error.
void float_ser(std::ofstream& file, float f) {
    file << (unsigned char)f;
    file << (unsigned char)(256.0*f);
}

float float_deser(unsigned char high, unsigned char low) {
    return (float)high + (float)low/256.0;
}

FPoint::FPoint(float ax, float ay): x {ax}, y {ay} {}

FPoint::FPoint(const Point& p): x {static_cast<float>(p.x)}, y {static_cast<float>(p.y)} {}

CameraContext::CameraContext(int x, int y, int w, int h, int priority): x_ {x}, y_ {y}, w_ {w}, h_ {h}, priority_ {priority} {}

CameraContext::~CameraContext() {}

bool CameraContext::is_null() {
    return false;
}

void CameraContext::serialize(std::ofstream& file) {
    file << (unsigned char)x_;
    file << (unsigned char)y_;
    file << (unsigned char)w_;
    file << (unsigned char)h_;
    file << (unsigned char)priority_;
}

FreeCameraContext::FreeCameraContext(int x, int y, int w, int h, int priority, float radius):
CameraContext(x, y, w, h, priority), radius_ {radius} {}

FreeCameraContext::~FreeCameraContext() {}

FPoint FreeCameraContext::center(Point pos) {
    return pos;
}

float FreeCameraContext::radius(Point pos) {
    return radius_;
}

void FreeCameraContext::serialize(std::ofstream& file) {
    file << (unsigned char)CameraCode::Free;
    CameraContext::serialize(file);
    float_ser(file, radius_);
}

CameraContext* FreeCameraContext::deserialize(unsigned char* b) {
    return new FreeCameraContext(b[0], b[1], b[2], b[3], b[4],
                                 float_deser(b[5], b[6]));
}

FixedCameraContext::FixedCameraContext(int x, int y, int w, int h, int priority, float radius, float cx, float cy):
CameraContext(x, y, w, h, priority), radius_ {radius}, cx_ {cx}, cy_ {cy} {}

FixedCameraContext::~FixedCameraContext() {}

FPoint FixedCameraContext::center(Point pos) {
    return FPoint{cx_, cy_};
}

float FixedCameraContext::radius(Point pos) {
    return radius_;
}

void FixedCameraContext::serialize(std::ofstream& file) {
    file << (unsigned char)CameraCode::Fixed;
    CameraContext::serialize(file);
    float_ser(file, radius_);
    float_ser(file, cx_);
    float_ser(file, cy_);
}

CameraContext* FixedCameraContext::deserialize(unsigned char* b) {
    return new FixedCameraContext(b[0], b[1], b[2], b[3], b[4],
                                  float_deser(b[5], b[6]),
                                  float_deser(b[7], b[8]), float_deser(b[9], b[10]));
}

ClampedCameraContext::ClampedCameraContext(int x, int y, int w, int h, int priority, float radius, int xpad, int ypad):
CameraContext(x, y, w, h, priority), radius_ {radius}, xpad_ {xpad}, ypad_ {ypad} {}

ClampedCameraContext::~ClampedCameraContext() {}

FPoint ClampedCameraContext::center(Point pos) {
    return Point{
        std::min(std::max(pos.x, x_ + xpad_), x_ + w_ - xpad_),
        std::min(std::max(pos.y, y_ + ypad_), y_ + h_ - ypad_)
    };
}

float ClampedCameraContext::radius(Point pos) {
    return radius_;
}

void ClampedCameraContext::serialize(std::ofstream& file) {
    file << (unsigned char)CameraCode::Clamped;
    CameraContext::serialize(file);
    float_ser(file, radius_);
    file << (unsigned char) xpad_;
    file << (unsigned char) ypad_;
}

CameraContext* ClampedCameraContext::deserialize(unsigned char* b) {
    return new ClampedCameraContext(b[0], b[1], b[2], b[3], b[4],
                                    float_deser(b[5], b[6]),
                                    b[7], b[8]);
}

NullCameraContext::NullCameraContext(int x, int y, int w, int h, int priority):
    CameraContext(x, y, w, h, priority) {}

NullCameraContext::~NullCameraContext() {}

bool NullCameraContext::is_null() {
    return true;
}

FPoint NullCameraContext::center(Point pos) {
    return pos;
}

float NullCameraContext::radius(Point pos) {
    return DEFAULT_CAM_RADIUS;
}

void NullCameraContext::serialize(std::ofstream& file) {
    file << (unsigned char)CameraCode::Null;
    CameraContext::serialize(file);
}

CameraContext* NullCameraContext::deserialize(unsigned char* b) {
    return new NullCameraContext(b[0], b[1], b[2], b[3], b[4]);
}

Camera::Camera(RoomMap* room_map): width_ {room_map->width()}, height_ {room_map->height()},
    default_context_ {FreeCameraContext(0, 0, room_map->width(), room_map->height(), 0, DEFAULT_CAM_RADIUS)},
    context_ {}, loaded_contexts_ {},
    context_map_ {},
    target_rad_ {DEFAULT_CAM_RADIUS}, cur_rad_ {DEFAULT_CAM_RADIUS},
    target_pos_ {FPoint{0,0}}, cur_pos_ {FPoint{0,0}}
{
    context_map_ = std::vector<std::vector<CameraContext*>>(room_map->width(), std::vector<CameraContext*>(room_map->height(), &default_context_));
}

void Camera::serialize(std::ofstream& file) {
    file << static_cast<unsigned char>(State::CameraRect);
    for (auto& context : loaded_contexts_) {
        context->serialize(file);
    }
    file << static_cast<unsigned char>(CameraCode::NONE);
}

void Camera::push_context(std::unique_ptr<CameraContext> context) {
    int left = context->x_;
    int right = left + context->w_;
    int top = context->y_;
    int bottom = top + context->h_;
    int priority = context->priority_;
    for (int i = left; i < right; ++i) {
        for (int j = top; j < bottom; ++j) {
            if (priority > context_map_[i][j]->priority_) {
                context_map_[i][j] = context.get();
            }
        }
    }
    loaded_contexts_.push_back(std::move(context));
    auto c = static_cast<FixedCameraContext*>(loaded_contexts_.back().get());
}


float Camera::get_radius() {
    return cur_rad_;
}

FPoint Camera::get_pos() {
    return cur_pos_;
}

void Camera::set_target(Point pos) {
    CameraContext* new_context = context_map_[pos.x][pos.y];
    if (!new_context->is_null()) {
        context_ = new_context;
    }
    target_pos_ = context_->center(pos);
    target_rad_ = context_->radius(pos);
}

void Camera::set_current_pos(Point pos) {
    target_pos_ = pos;
    cur_pos_ = pos;
}

void Camera::update() {
    cur_pos_ = FPoint{damp_avg(target_pos_.x, cur_pos_.x), damp_avg(target_pos_.y, cur_pos_.y)};
    cur_rad_ = damp_avg(target_rad_, cur_rad_);
}

// We have a few magic numbers for tweaking camera smoothness
// This function may be something more interesting than exponential damping later
float damp_avg(float target, float cur) {
    if (fabs(target - cur) <= 0.0001) {
        return target;
    } else {
        return (target + 8*cur)/9.0;
    }
}
