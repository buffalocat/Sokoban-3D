#include "camera.h"
#include "worldmap.h"
#include <algorithm>

FPoint::FPoint(float ax, float ay): x {ax}, y {ay} {}

FPoint::FPoint(const Point& p): x {static_cast<float>(p.x)}, y {static_cast<float>(p.y)} {}

CameraContext::CameraContext(int x, int y, int w, int h, int priority): x_ {x}, y_ {y}, w_ {w}, h_ {h}, priority_ {priority} {}

CameraContext::~CameraContext() {}

bool CameraContext::is_null() {
    return false;
}

FreeCameraContext::FreeCameraContext(int x, int y, int w, int h, int priority):
CameraContext(x, y, w, h, priority), radius_ {} {
    radius_ = DEFAULT_CAM_RADIUS;
}

FreeCameraContext::~FreeCameraContext() {}

Point FreeCameraContext::center(Point pos) {
    return pos;
}

float FreeCameraContext::radius(Point pos) {
    return radius_;
}

FixedCameraContext::FixedCameraContext(int x, int y, int w, int h, int priority, float radius, int cx, int cy):
CameraContext(x, y, w, h, priority), cx_ {cx}, cy_ {cy}, radius_ {radius} {
    //radius_ = DEFAULT_CAM_RADIUS;
}

FixedCameraContext::~FixedCameraContext() {}

Point FixedCameraContext::center(Point pos) {
    return Point{cx_, cy_};
}

float FixedCameraContext::radius(Point pos) {
    return radius_;
}

ClampedCameraContext::ClampedCameraContext(int x, int y, int w, int h, int priority, int xpad, int ypad):
CameraContext(x, y, w, h, priority), xpad_ {xpad}, ypad_ {ypad}, radius_ {} {
    radius_ = DEFAULT_CAM_RADIUS;
}

ClampedCameraContext::~ClampedCameraContext() {}

Point ClampedCameraContext::center(Point pos) {
    return Point{
        std::min(std::max(pos.x, x_ + xpad_), x_ + w_ - xpad_),
        std::min(std::max(pos.y, y_ + ypad_), y_ + h_ - ypad_)
    };
}

float ClampedCameraContext::radius(Point pos) {
    return radius_;
}

NullCameraContext::NullCameraContext(int x, int y, int w, int h, int priority):
    CameraContext(x, y, w, h, priority) {}

NullCameraContext::~NullCameraContext() {}

bool NullCameraContext::is_null() {
    return true;
}

Point NullCameraContext::center(Point pos) {
    return pos;
}

float NullCameraContext::radius(Point pos) {
    return DEFAULT_CAM_RADIUS;
}


Camera::Camera(WorldMap* world_map, float rad, Point pos): context_ {}, loaded_contexts_ {}, context_map_ {}, target_rad_ {rad}, cur_rad_ {rad}, target_pos_ {pos}, cur_pos_ {pos} {
    auto default_context = std::make_unique<FreeCameraContext>(0, 0, world_map->width(), world_map->height(), 0);
    context_map_ = std::vector<std::vector<CameraContext*>>(world_map->width(), std::vector<CameraContext*>(world_map->height(), default_context.get()));
    loaded_contexts_.push_back(std::move(default_context));
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

void Camera::update() {
    cur_pos_ = FPoint{damp_avg(target_pos_.x, cur_pos_.x), damp_avg(target_pos_.y, cur_pos_.y)};
    cur_rad_ = damp_avg(target_rad_, cur_rad_);
}

// We have a few magic numbers for tweaking camera smoothness
// This function may be something more interesting than exponential damping later
float damp_avg(float target, float cur) {
    if (fabs(target - cur) <= 0.01) {
        return target;
    } else {
        return (target + 8*cur)/9.0;
    }
}
