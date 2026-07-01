#pragma once
#include "hittable.h"

class Sphere : public Hittable {
public:
    Point3 center;
    float  radius;
    std::shared_ptr<Material> mat;

    Sphere(const Point3& c, float r, std::shared_ptr<Material> m)
        : center(c), radius(r), mat(std::move(m)) {}

    bool hit(const Ray& r, float t_min, float t_max, HitRecord& rec) const override {
        Vec3  oc     = r.origin - center;
        float a      = r.dir.length_squared();
        float half_b = dot(oc, r.dir);
        float c      = oc.length_squared() - radius * radius;
        float disc   = half_b * half_b - a * c;
        if (disc < 0.0f) return false;
        float sq = std::sqrt(disc);
        float root = (-half_b - sq) / a;
        if (root < t_min || root > t_max) {
            root = (-half_b + sq) / a;
            if (root < t_min || root > t_max) return false;
        }
        rec.t = root;
        rec.p = r.at(root);
        Vec3 outward = (rec.p - center) / radius;
        rec.set_face_normal(r, outward);
        float theta = std::acos(clampf(-outward.y, -1.0f, 1.0f));
        float phi   = std::atan2(-outward.z, outward.x) + PI;
        rec.u = phi / (2.0f * PI);
        rec.v = theta / PI;
        rec.mat = mat;
        return true;
    }

    bool bounding_box(AABB& output_box) const override {
        output_box = AABB(center - Vec3(radius, radius, radius),
                          center + Vec3(radius, radius, radius));
        return true;
    }
};
