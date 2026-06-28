#pragma once
// ---------------------------------------------------------------------------
// hittable.h — the interface every renderable surface implements, plus the
// HitRecord that a successful intersection fills in. HittableList is a naive
// "test every object" container; Phase 1 Step 6 swaps it for a BVH.
// ---------------------------------------------------------------------------
#include "ray.h"
#include <memory>
#include <vector>

class Material;   // forward declaration — HitRecord only stores a pointer

struct HitRecord {
    Point3 p;          // world-space hit point
    Vec3   normal;     // surface normal at p (always faces the incoming ray)
    float  t;          // ray parameter at the hit
    float  u, v;       // texture coordinates
    bool   front_face; // did we hit the outside of the surface?
    std::shared_ptr<Material> mat;

    // Make the stored normal always point against the ray, and remember which
    // side we hit. Pass the geometric outward-facing normal.
    void set_face_normal(const Ray& r, const Vec3& outward_normal) {
        front_face = dot(r.dir, outward_normal) < 0.0f;
        normal     = front_face ? outward_normal : -outward_normal;
    }
};

class Hittable {
public:
    virtual ~Hittable() = default;
    virtual bool hit(const Ray& r, float t_min, float t_max, HitRecord& rec) const = 0;
};

// Simple linear container: returns the closest hit among all objects.
class HittableList : public Hittable {
public:
    std::vector<std::shared_ptr<Hittable>> objects;

    void clear() { objects.clear(); }
    void add(std::shared_ptr<Hittable> object) { objects.push_back(std::move(object)); }

    bool hit(const Ray& r, float t_min, float t_max, HitRecord& rec) const override {
        HitRecord temp;
        bool  hit_anything = false;
        float closest      = t_max;
        for (const auto& object : objects) {
            if (object->hit(r, t_min, closest, temp)) {
                hit_anything = true;
                closest      = temp.t;   // shrink the window: only accept nearer hits
                rec          = temp;
            }
        }
        return hit_anything;
    }
};
