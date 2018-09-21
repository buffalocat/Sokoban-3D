#ifndef CAMERA_H
#define CAMERA_H

#include "common.h"

#include <queue>

class RoomMap;

struct FPoint {
    float x;
    float y;
    FPoint(float, float);
    FPoint(const Point&);
};

class CameraContext {
public:
    CameraContext(int x, int y, int w, int h, int priority);
    virtual ~CameraContext() = 0;
    virtual Point center(Point) = 0;
    virtual float radius(Point) = 0;
    virtual bool is_null();

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
    FreeCameraContext(int x, int y, int w, int h, int priority);
    ~FreeCameraContext();
    Point center(Point);
    float radius(Point);

private:
    float radius_;
};

class FixedCameraContext: public CameraContext {
public:
    FixedCameraContext(int x, int y, int w, int h, int priority, float radius, int cx, int cy);
    ~FixedCameraContext();
    Point center(Point);
    float radius(Point);

private:
    int cx_;
    int cy_;
    float radius_;
};

class ClampedCameraContext: public CameraContext {
public:
    ClampedCameraContext(int x, int y, int w, int h, int priority, int xpad, int ypad);
    ~ClampedCameraContext();
    Point center(Point);
    float radius(Point);

private:
    int xpad_;
    int ypad_;
    float radius_;
};

class NullCameraContext: public CameraContext {
public:
    NullCameraContext(int x, int y, int w, int h, int priority);
    ~NullCameraContext();
    bool is_null();
    Point center(Point);
    float radius(Point);
};

class Camera {
public:
    Camera(RoomMap*);
    void update();
    void set_target(Point);
    void set_current_pos(Point);
    float get_radius();
    FPoint get_pos();
    void push_context(std::unique_ptr<CameraContext>);

private:
    int width_;
    int height_;
    FreeCameraContext default_context_;
    CameraContext* context_;
    std::vector<std::unique_ptr<CameraContext>> loaded_contexts_;
    std::vector<std::vector<CameraContext*>> context_map_;
    float target_rad_;
    float cur_rad_;
    FPoint target_pos_;
    FPoint cur_pos_;
};

float damp_avg(float target, float cur);


#endif // CAMERA_H
