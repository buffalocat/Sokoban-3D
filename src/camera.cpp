#include <cmath>

#include "camera.h"
#include "roommap.h"
#include "mapfile.h"

CameraContext::CameraContext(int x, int y, int w, int h, int priority): x_ {x}, y_ {y}, w_ {w}, h_ {h}, priority_ {priority} {}

CameraContext::~CameraContext() {}

bool CameraContext::is_null() {
    return false;
}

FPoint3 CameraContext::center(FPoint3 pos) {
    return pos;
}

float CameraContext::radius(FPoint3 pos) {
    return DEFAULT_CAM_RADIUS;
}

float CameraContext::tilt(FPoint3 pos) {
    return DEFAULT_CAM_TILT;
}

float CameraContext::rotation(FPoint3 pos) {
    return DEFAULT_CAM_ROTATION;
}

void CameraContext::serialize(MapFileO& file) {
    file << x_;
    file << y_;
    file << w_;
    file << h_;
    file << priority_;
}

FreeCameraContext::FreeCameraContext(int x, int y, int w, int h, int priority, float radius, float tilt, float rotation):
CameraContext(x, y, w, h, priority), radius_ {radius}, tilt_ {tilt}, rotation_ {rotation} {}

FreeCameraContext::~FreeCameraContext() {}

FPoint3 FreeCameraContext::center(FPoint3 pos) {
    return pos;
}

float FreeCameraContext::radius(FPoint3 pos) {
    return radius_;
}

float FreeCameraContext::tilt(FPoint3 pos) {
    return tilt_;
}

float FreeCameraContext::rotation(FPoint3 pos) {
    return rotation_;
}

void FreeCameraContext::serialize(MapFileO& file) {
    file << CameraCode::Free;
    CameraContext::serialize(file);
    file << radius_;
    file << tilt_;
    file << rotation_;
}

CameraContext* FreeCameraContext::deserialize(MapFileI& file) {
    int x, y, w, h, p;
    float rad, tilt, rot;
    file >> x >> y >> w >> h >> p >> rad >> tilt >> rot;
    return new FreeCameraContext(x, y, w, h, p, rad, tilt, rot);
}


FixedCameraContext::FixedCameraContext(int x, int y, int w, int h, int priority, float radius, float tilt, float rotation, FPoint3 center):
CameraContext(x, y, w, h, priority), radius_ {radius}, tilt_ {tilt}, rotation_ {rotation}, center_ {center} {}

FixedCameraContext::~FixedCameraContext() {}

FPoint3 FixedCameraContext::center(FPoint3 pos) {
    return center_;
}

float FixedCameraContext::radius(FPoint3 pos) {
    return radius_;
}

float FixedCameraContext::tilt(FPoint3 pos) {
    return tilt_;
}

float FixedCameraContext::rotation(FPoint3 pos) {
    return rotation_;
}

void FixedCameraContext::serialize(MapFileO& file) {
    file << CameraCode::Fixed;
    CameraContext::serialize(file);
    file << radius_;
    file << tilt_;
    file << rotation_;
    file << center_;
}

CameraContext* FixedCameraContext::deserialize(MapFileI& file) {
    int x, y, w, h, p;
    float rad, tilt, rot;
    FPoint3 center {};
    file >> x >> y >> w >> h >> p >> rad >> tilt >> rot >> center;
    return new FixedCameraContext(x, y, w, h, p, rad, tilt, rot, center);
}

ClampedCameraContext::ClampedCameraContext(int x, int y, int w, int h, int priority, float radius, float tilt, int xpad, int ypad):
CameraContext(x, y, w, h, priority), radius_ {radius}, xpad_ {xpad}, ypad_ {ypad} {}

ClampedCameraContext::~ClampedCameraContext() {}

FPoint3 ClampedCameraContext::center(FPoint3 pos) {
    return {
        std::min(std::max(pos.x, (float)x_ + xpad_), (float)x_ + w_ - xpad_),
        std::min(std::max(pos.y, (float)y_ + ypad_), (float)y_ + h_ - ypad_),
        pos.z
    };
}

float ClampedCameraContext::radius(FPoint3 pos) {
    return radius_;
}

float ClampedCameraContext::tilt(FPoint3 pos) {
    return tilt_;
}

void ClampedCameraContext::serialize(MapFileO& file) {
    file << CameraCode::Clamped;
    CameraContext::serialize(file);
    file << radius_;
    file << tilt_;
    file << xpad_;
    file << ypad_;
}

CameraContext* ClampedCameraContext::deserialize(MapFileI& file) {
    int x, y, w, h, p, xpad, ypad;
    float rad, tilt;
    file >> x >> y >> w >> h >> p >> rad >> tilt >> xpad >> ypad;
    return new ClampedCameraContext(x, y, w, h, p, rad, tilt, xpad, ypad);
}

NullCameraContext::NullCameraContext(int x, int y, int w, int h, int priority):
    CameraContext(x, y, w, h, priority) {}

NullCameraContext::~NullCameraContext() {}

bool NullCameraContext::is_null() {
    return true;
}

void NullCameraContext::serialize(MapFileO& file) {
    file << (unsigned char)CameraCode::Null;
    CameraContext::serialize(file);
}

CameraContext* NullCameraContext::deserialize(MapFileI& file) {
    int x, y, w, h, p;
    file >> x >> y >> w >> h >> p;
    return new NullCameraContext(x, y, w, h, p);
}


Camera::Camera(int w, int h): width_ {w}, height_ {h},
    default_context_ {FreeCameraContext(0, 0, w, h, 0, DEFAULT_CAM_RADIUS, DEFAULT_CAM_TILT, DEFAULT_CAM_ROTATION)},
    context_ {}, loaded_contexts_ {},
    context_map_ {},
    target_pos_ {FPoint3{0,0,0}}, cur_pos_ {FPoint3{0,0,0}},
    target_rad_ {DEFAULT_CAM_RADIUS}, cur_rad_ {DEFAULT_CAM_RADIUS},
    target_tilt_ {DEFAULT_CAM_TILT}, cur_tilt_ {DEFAULT_CAM_TILT},
    target_rotation_ {DEFAULT_CAM_ROTATION}, cur_rotation_ {DEFAULT_CAM_ROTATION}
{
    context_ = &default_context_;
    context_map_ = std::vector<std::vector<CameraContext*>>(w, std::vector<CameraContext*>(h, context_));
}

void Camera::serialize(MapFileO& file) {
    file << MapCode::CameraRects;
    for (auto& context : loaded_contexts_) {
        context->serialize(file);
    }
    file << CameraCode::NONE;
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

FPoint3 Camera::get_pos() {
    return cur_pos_;
}

float Camera::get_tilt() {
    return cur_tilt_;
}

float Camera::get_rotation() {
    return cur_rotation_;
}

void Camera::set_target(Point3 vpos, FPoint3 rpos) {
    CameraContext* new_context = context_map_[vpos.x][vpos.y];
    if (!new_context->is_null()) {
        context_ = new_context;
    }
    target_pos_ = context_->center(rpos);
    target_rad_ = context_->radius(rpos);
    target_tilt_ = context_->tilt(rpos);
    target_rotation_ = context_->rotation(rpos);
}

void Camera::set_current_pos(FPoint3 pos) {
    target_pos_ = pos;
    cur_pos_ = pos;
    target_rad_ = context_->radius(pos);
    cur_rad_ = target_rad_;
    target_tilt_ = context_->tilt(pos);
    cur_tilt_ = target_tilt_;
    target_rotation_ = context_->rotation(pos);
    cur_rotation_ = target_rotation_;
}

void Camera::update() {
    cur_pos_ = FPoint3{damp_avg(target_pos_.x, cur_pos_.x), damp_avg(target_pos_.y, cur_pos_.y), damp_avg(target_pos_.z, cur_pos_.z)};
    cur_rad_ = damp_avg(target_rad_, cur_rad_);
    cur_tilt_ = damp_avg(target_tilt_, cur_tilt_);
    cur_rotation_ = damp_avg(target_rotation_, cur_rotation_);
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
