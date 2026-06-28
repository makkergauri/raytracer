#pragma once
// ---------------------------------------------------------------------------
// camera.h — generates the primary rays we shoot through each pixel.
//
//   * vfov + aspect define the viewport size.
//   * lookfrom/lookat/vup orient the camera (u,v,w form its basis).
//   * aperture > 0 enables depth of field: rays start from a random point on
//     a lens of that diameter, so only the focus plane stays sharp.
//   * time0/time1 let each ray pick a random shutter instant (motion blur).
//
// Animation note: in the frame pipeline we simply rebuild the Camera every
// frame from keyframed lookfrom/lookat values — no special machinery needed.
// ---------------------------------------------------------------------------
#include "ray.h"

class Camera {
public:
    Camera(Point3 lookfrom, Point3 lookat, Vec3 vup,
           float vfov_degrees, float aspect_ratio,
           float aperture = 0.0f, float focus_dist = 1.0f,
           float t0 = 0.0f, float t1 = 0.0f) {
        float theta           = vfov_degrees * DEG2RAD;
        float h               = std::tan(theta / 2.0f);
        float viewport_height = 2.0f * h;
        float viewport_width  = aspect_ratio * viewport_height;

        w = normalize(lookfrom - lookat);   // points back toward the camera
        u = normalize(cross(vup, w));        // camera right
        v = cross(w, u);                     // camera up

        origin     = lookfrom;
        horizontal = focus_dist * viewport_width  * u;
        vertical   = focus_dist * viewport_height * v;
        lower_left = origin - horizontal / 2.0f - vertical / 2.0f - focus_dist * w;

        lens_radius = aperture / 2.0f;
        time0 = t0;
        time1 = t1;
    }

    // s,t are normalized screen coordinates in [0,1].
    Ray generate_ray(float s, float t) const {
        Vec3   rd     = lens_radius * random_in_unit_disk();
        Vec3   offset = u * rd.x + v * rd.y;          // jitter the ray origin on the lens
        Point3 ray_origin = origin + offset;
        Vec3   dir = lower_left + s * horizontal + t * vertical - ray_origin;
        float  ray_time = (time1 > time0) ? random_float(time0, time1) : time0;
        return Ray(ray_origin, dir, ray_time);
    }

private:
    Point3 origin, lower_left;
    Vec3   horizontal, vertical;
    Vec3   u, v, w;
    float  lens_radius;
    float  time0, time1;
};
