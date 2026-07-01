#pragma once
// Triangle primitive with Moller-Trumbore intersection and smooth (barycentric) normals.
#include "hittable.h"

class Triangle : public Hittable {
public:
    Point3 v0, v1, v2;
    Vec3   n0, n1, n2;
    std::shared_ptr<Material> mat;

    Triangle(const Point3& a, const Point3& b, const Point3& c, std::shared_ptr<Material> m)
        : v0(a), v1(b), v2(c), mat(std::move(m)) {
        Vec3 fn = normalize(cross(v1 - v0, v2 - v0));
        n0 = n1 = n2 = fn;
    }
    Triangle(const Point3& a, const Point3& b, const Point3& c,
             const Vec3& na, const Vec3& nb, const Vec3& nc, std::shared_ptr<Material> m)
        : v0(a), v1(b), v2(c), n0(na), n1(nb), n2(nc), mat(std::move(m)) {}

    bool hit(const Ray& r, float t_min, float t_max, HitRecord& rec) const override {
        Vec3  e1 = v1 - v0, e2 = v2 - v0;
        Vec3  p  = cross(r.dir, e2);
        float det = dot(e1, p);
        if (std::fabs(det) < 1e-9f) return false;
        float inv = 1.0f / det;
        Vec3  tv = r.origin - v0;
        float u  = dot(tv, p) * inv;
        if (u < 0.0f || u > 1.0f) return false;
        Vec3  q  = cross(tv, e1);
        float v  = dot(r.dir, q) * inv;
        if (v < 0.0f || u + v > 1.0f) return false;
        float t  = dot(e2, q) * inv;
        if (t < t_min || t > t_max) return false;

        rec.t = t;
        rec.p = r.at(t);
        float w = 1.0f - u - v;
        Vec3 nrm = normalize(w * n0 + u * n1 + v * n2);
        rec.set_face_normal(r, nrm);
        rec.u = u; rec.v = v;
        rec.mat = mat;
        return true;
    }

    bool bounding_box(AABB& output_box) const override {
        Vec3 mn(std::fmin(v0.x, std::fmin(v1.x, v2.x)),
                std::fmin(v0.y, std::fmin(v1.y, v2.y)),
                std::fmin(v0.z, std::fmin(v1.z, v2.z)));
        Vec3 mx(std::fmax(v0.x, std::fmax(v1.x, v2.x)),
                std::fmax(v0.y, std::fmax(v1.y, v2.y)),
                std::fmax(v0.z, std::fmax(v1.z, v2.z)));
        Vec3 eps(1e-4f, 1e-4f, 1e-4f);
        output_box = AABB(mn - eps, mx + eps);
        return true;
    }
};
