#pragma once
#include "ray.h"
#include "aabb.h"
#include <memory>
#include <vector>

class Material;

struct HitRecord {
    Point3 p;
    Vec3   normal;
    float  t;
    float  u, v;
    bool   front_face;
    std::shared_ptr<Material> mat;

    void set_face_normal(const Ray& r, const Vec3& outward_normal) {
        front_face = dot(r.dir, outward_normal) < 0.0f;
        normal     = front_face ? outward_normal : -outward_normal;
    }
};

class Hittable {
public:
    virtual ~Hittable() = default;
    virtual bool hit(const Ray& r, float t_min, float t_max, HitRecord& rec) const = 0;
    virtual bool bounding_box(AABB& output_box) const = 0;
};

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
                closest      = temp.t;
                rec          = temp;
            }
        }
        return hit_anything;
    }

    bool bounding_box(AABB& output_box) const override {
        if (objects.empty()) return false;
        AABB temp; bool first = true;
        for (const auto& object : objects) {
            if (!object->bounding_box(temp)) return false;
            output_box = first ? temp : surrounding_box(output_box, temp);
            first = false;
        }
        return true;
    }
};
