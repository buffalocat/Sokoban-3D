#ifndef CAMERA_H
#define CAMERA_H

#include "common.h"

class RoomMap;
class MapFileI;
class MapFileO;

class CameraContext {
public:
    CameraContext(int x, int y, int w, int h, int priority);
    virtual ~CameraContext();
    virtual bool is_null();
    virtual FPoint3 center(Point3) = 0;
    virtual float radius(Point3) = 0;
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
    FreeCameraContext(int x, int y, int w, int h, int priority, float radius);
    ~FreeCameraContext();
    FPoint3 center(Point3);
    float radius(Point3);
    void serialize(MapFileO& file);
    //static CameraContext* deserialize(unsigned char* buffer);

private:
    float radius_;
};

class FixedCameraContext: public CameraContext {
public:
    FixedCameraContext(int x, int y, int w, int h, int priority, float radius, FPoint3);
    ~FixedCameraContext();
    FPoint3 center(Point3);
    float radius(Point3);
    void serialize(MapFileO& file);
    //static CameraContext* deserialize(MapFileI& file);

private:
    float radius_;
    FPoint3 center_;
    friend class Camera;
};

class ClampedCameraContext: public CameraContext {
public:
    ClampedCameraContext(int x, int y, int w, int h, int priority, float radius, int xpad, int ypad);
    ~ClampedCameraContext();
    FPoint3 center(Point3);
    float radius(Point3);
    void serialize(MapFileO& file);
    //static CameraContext* deserialize(unsigned char* buffer);

private:
    float radius_;
    int xpad_;
    int ypad_;
};

class NullCameraContext: public CameraContext {
public:
    NullCameraContext(int x, int y, int w, int h, int priority);
    ~NullCameraContext();
    bool is_null();
    FPoint3 center(Point3);
    float radius(Point3);
    void serialize(MapFileO& file);
    //static CameraContext* deserialize(unsigned char* buffer);
};

class Camera {
public:
    Camera(int w, int h);
    void serialize(MapFileO& file);
    void update();
    void set_target(Point3);
    void set_current_pos(Point3);
    float get_radius();
    FPoint3 get_pos();
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
    FPoint3 target_pos_;
    FPoint3 cur_pos_;
};

float damp_avg(float target, float cur);


#endif // CAMERA_H
