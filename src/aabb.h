#pragma once
// Axis-aligned bounding box + slab intersection test (used by the BVH).
#include "ray.h"
#include <algorithm>

inline float vcomp(const Vec3& v, int a) { return a == 0 ? v.x : (a == 1 ? v.y : v.z); }

struct AABB {
    Vec3 mn, mx;
    AABB() : mn(Vec3(INF, INF, INF)), mx(Vec3(-INF, -INF, -INF)) {}
    AABB(const Vec3& a, const Vec3& b) : mn(a), mx(b) {}

    bool hit(const Ray& r, float tmin, float tmax) const {
        for (int a = 0; a < 3; ++a) {
            float invD = 1.0f / vcomp(r.dir, a);
            float t0 = (vcomp(mn, a) - vcomp(r.origin, a)) * invD;
            float t1 = (vcomp(mx, a) - vcomp(r.origin, a)) * invD;
            if (invD < 0.0f) std::swap(t0, t1);
            tmin = t0 > tmin ? t0 : tmin;
            tmax = t1 < tmax ? t1 : tmax;
            if (tmax <= tmin) return false;
        }
        return true;
    }
};

inline AABB surrounding_box(const AABB& a, const AABB& b) {
    Vec3 small(std::fmin(a.mn.x, b.mn.x), std::fmin(a.mn.y, b.mn.y), std::fmin(a.mn.z, b.mn.z));
    Vec3 big  (std::fmax(a.mx.x, b.mx.x), std::fmax(a.mx.y, b.mx.y), std::fmax(a.mx.z, b.mx.z));
    return AABB(small, big);
}
