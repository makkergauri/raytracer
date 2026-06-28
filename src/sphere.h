#pragma once
// ---------------------------------------------------------------------------
// sphere.h — analytic ray/sphere intersection.
//
// A point P is on the sphere when |P - C|^2 = r^2. Substituting the ray
// P = O + t*D and expanding gives a quadratic in t:
//     (D.D) t^2 + 2 (D.(O-C)) t + ((O-C).(O-C) - r^2) = 0
// We solve it and keep the nearest root inside [t_min, t_max].
// (MovingSphere for motion blur is added in Phase 2.)
// ---------------------------------------------------------------------------
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

        float discriminant = half_b * half_b - a * c;
        if (discriminant < 0.0f) return false;
        float sqrtd = std::sqrt(discriminant);

        // Find the nearest root in the acceptable range.
        float root = (-half_b - sqrtd) / a;
        if (root < t_min || root > t_max) {
            root = (-half_b + sqrtd) / a;
            if (root < t_min || root > t_max) return false;
        }

        rec.t = root;
        rec.p = r.at(root);
        Vec3 outward_normal = (rec.p - center) / radius;
        rec.set_face_normal(r, outward_normal);
        get_sphere_uv(outward_normal, rec.u, rec.v);
        rec.mat = mat;
        return true;
    }

private:
    // Map a point on the unit sphere to (u,v) in [0,1] using spherical coords.
    static void get_sphere_uv(const Point3& p, float& u, float& v) {
        float theta = std::acos(clampf(-p.y, -1.0f, 1.0f));
        float phi   = std::atan2(-p.z, p.x) + PI;
        u = phi / (2.0f * PI);
        v = theta / PI;
    }
};
