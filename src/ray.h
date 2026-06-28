#pragma once
// ---------------------------------------------------------------------------
// ray.h — a ray is a point + a direction. The `time` field lets a single ray
// be fired at a random instant during the shutter interval (motion blur).
// ---------------------------------------------------------------------------
#include "vec3.h"

struct Ray {
    Point3 origin;
    Vec3   dir;
    float  time;   // shutter time in [0,1]; 0 when motion blur is off

    Ray() : time(0.0f) {}
    Ray(const Point3& o, const Vec3& d, float t = 0.0f) : origin(o), dir(d), time(t) {}

    // The point you reach after travelling distance `t` along the ray.
    Point3 at(float t) const { return origin + t * dir; }
};
