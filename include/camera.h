#ifndef CAMERA_H
#define CAMERA_H

#include <memory>
#include <vector>

#include "point.h"

class RoomMap;
class MapFileI;
class MapFileO;

class CameraContext {
public:
    CameraContext(int x, int y, int w, int h, int priority);
    virtual ~CameraContext() = 0;
    virtual bool is_null();
    virtual FPoint3 center(FPoint3);
    virtual float radius(FPoint3);
    virtual float tilt(FPoint3);
    virtual float rotation(FPoint3);
    virtual void serialize(MapFileO& file);

protected:
    int x_;
    int y_;
    int w_;
    int h_;
    int priority_;

    friend class Camera;
};

class FreeCameraContext: public CameraContext {
public:
    FreeCameraContext(int x, int y, int w, int h, int priority, float radius, float tilt, float rotation);
    ~FreeCameraContext();
    FPoint3 center(FPoint3);
    float radius(FPoint3);
    float tilt(FPoint3);
    float rotation(FPoint3);
    void serialize(MapFileO& file);
    static CameraContext* deserialize(MapFileI& file);

private:
    float radius_;
    float tilt_;
    float rotation_;
};

class FixedCameraContext: public CameraContext {
public:
    FixedCameraContext(int x, int y, int w, int h, int priority, float radius, float tilt, float rotation, FPoint3 center);
    ~FixedCameraContext();
    FPoint3 center(FPoint3);
    float radius(FPoint3);
    float tilt(FPoint3);
    float rotation(FPoint3);
    void serialize(MapFileO& file);
    static CameraContext* deserialize(MapFileI& file);

private:
    float radius_;
    float tilt_;
    float rotation_;
    FPoint3 center_;
    friend class Camera;
};

class ClampedCameraContext: public CameraContext {
public:
    ClampedCameraContext(int x, int y, int w, int h, int priority, float radius, float tilt, int xpad, int ypad);
    ~ClampedCameraContext();
    FPoint3 center(FPoint3);
    float radius(FPoint3);
    float tilt(FPoint3);
    void serialize(MapFileO& file);
    static CameraContext* deserialize(MapFileI& file);

private:
    float radius_;
    float tilt_;
    int xpad_;
    int ypad_;
};

class NullCameraContext: public CameraContext {
public:
    NullCameraContext(int x, int y, int w, int h, int priority);
    ~NullCameraContext();
    bool is_null();
    void serialize(MapFileO& file);
    static CameraContext* deserialize(MapFileI& file);
};

class Camera {
public:
    Camera(int w, int h);
    void serialize(MapFileO& file);
    void update();
    void set_target(Point3, FPoint3);
    void set_current_pos(FPoint3);
    float get_radius();
    FPoint3 get_pos();
    float get_tilt();
    float get_rotation();
    void push_context(std::unique_ptr<CameraContext>);

private:
    int width_;
    int height_;
    FreeCameraContext default_context_;
    CameraContext* context_;
    std::vector<std::unique_ptr<CameraContext>> loaded_contexts_;
    std::vector<std::vector<CameraContext*>> context_map_;
    FPoint3 target_pos_;
    FPoint3 cur_pos_;
    float target_rad_;
    float cur_rad_;
    float target_tilt_;
    float cur_tilt_;
    float target_rotation_;
    float cur_rotation_;
};

float damp_avg(float target, float cur);


#endif // CAMERA_H
