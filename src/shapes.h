#pragma once
// Analytic primitives for the bridge scene: finite Quad, axis-aligned Box,
// and an arbitrary-axis Cylinder. All implement hit() + bounding_box().
#include "hittable.h"
#include <algorithm>

// ---- Quad: a finite parallelogram (corner Q + edge vectors u,v) -----------
class Quad : public Hittable {
public:
    Point3 Q; Vec3 u, v;
    std::shared_ptr<Material> mat;
    Quad(const Point3& q, const Vec3& U, const Vec3& V, std::shared_ptr<Material> m)
        : Q(q), u(U), v(V), mat(std::move(m)) {
        Vec3 n = cross(u, v);
        normal = normalize(n);
        D = dot(normal, Q);
        w = n / dot(n, n);
    }
    bool hit(const Ray& r, float tmin, float tmax, HitRecord& rec) const override {
        float denom = dot(normal, r.dir);
        if (std::fabs(denom) < 1e-8f) return false;          // parallel
        float t = (D - dot(normal, r.origin)) / denom;
        if (t < tmin || t > tmax) return false;
        Point3 P = r.at(t);
        Vec3 planar = P - Q;
        float alpha = dot(w, cross(planar, v));
        float beta  = dot(w, cross(u, planar));
        if (alpha < 0 || alpha > 1 || beta < 0 || beta > 1) return false; // outside rect
        rec.t = t; rec.p = P;
        rec.set_face_normal(r, normal);   // two-sided: normal faces the ray
        rec.u = alpha; rec.v = beta; rec.mat = mat;
        return true;
    }
    bool bounding_box(AABB& out) const override {
        Point3 a = Q, b = Q + u, c = Q + v, d = Q + u + v;
        Vec3 mn(std::min({a.x,b.x,c.x,d.x}), std::min({a.y,b.y,c.y,d.y}), std::min({a.z,b.z,c.z,d.z}));
        Vec3 mx(std::max({a.x,b.x,c.x,d.x}), std::max({a.y,b.y,c.y,d.y}), std::max({a.z,b.z,c.z,d.z}));
        Vec3 eps(1e-3f,1e-3f,1e-3f);
        out = AABB(mn - eps, mx + eps);
        return true;
    }
private:
    Vec3 normal, w; float D;
};

// ---- Box: axis-aligned, analytic slab intersection with face normals ------
class Box : public Hittable {
public:
    Vec3 bmin, bmax;
    std::shared_ptr<Material> mat;
    Box(const Vec3& a, const Vec3& b, std::shared_ptr<Material> m) : mat(std::move(m)) {
        bmin = Vec3(std::fmin(a.x,b.x), std::fmin(a.y,b.y), std::fmin(a.z,b.z));
        bmax = Vec3(std::fmax(a.x,b.x), std::fmax(a.y,b.y), std::fmax(a.z,b.z));
    }
    bool hit(const Ray& r, float tmin, float tmax, HitRecord& rec) const override {
        float tNear = tmin, tFar = tmax; int axis = -1; float nsign = 0;
        for (int a = 0; a < 3; ++a) {
            float invd = 1.0f / vcomp(r.dir, a);
            float t0 = (vcomp(bmin,a) - vcomp(r.origin,a)) * invd;
            float t1 = (vcomp(bmax,a) - vcomp(r.origin,a)) * invd;
            float s = -1.0f;
            if (invd < 0.0f) { std::swap(t0,t1); s = 1.0f; }
            if (t0 > tNear) { tNear = t0; axis = a; nsign = s; }
            if (t1 < tFar) tFar = t1;
            if (tFar <= tNear) return false;
        }
        if (axis < 0) return false;
        rec.t = tNear; rec.p = r.at(tNear);
        Vec3 outward(0,0,0);
        if (axis == 0) outward.x = nsign; else if (axis == 1) outward.y = nsign; else outward.z = nsign;
        rec.set_face_normal(r, outward);
        rec.u = 0; rec.v = 0; rec.mat = mat;
        return true;
    }
    bool bounding_box(AABB& out) const override { out = AABB(bmin, bmax); return true; }
};

// ---- Cylinder: finite, between two endpoints p0,p1 (side surface) ---------
class Cylinder : public Hittable {
public:
    Point3 p0, p1; float radius;
    std::shared_ptr<Material> mat;
    Cylinder(const Point3& a, const Point3& b, float r, std::shared_ptr<Material> m)
        : p0(a), p1(b), radius(r), mat(std::move(m)) {
        axis = b - a; length = axis.length(); axis = axis / length;
    }
    bool hit(const Ray& r, float tmin, float tmax, HitRecord& rec) const override {
        Vec3 dp = r.dir - dot(r.dir, axis) * axis;        // ray dir perp to axis
        Vec3 oc = r.origin - p0;
        Vec3 op = oc - dot(oc, axis) * axis;              // origin offset perp
        float a = dot(dp, dp);
        if (a < 1e-12f) return false;                     // ray parallel to axis
        float b = 2.0f * dot(dp, op);
        float c = dot(op, op) - radius * radius;
        float disc = b*b - 4*a*c;
        if (disc < 0.0f) return false;
        float sq = std::sqrt(disc);
        for (float t : { (-b - sq)/(2*a), (-b + sq)/(2*a) }) {
            if (t < tmin || t > tmax) continue;
            Point3 P = r.at(t);
            float m = dot(P - p0, axis);
            if (m < 0.0f || m > length) continue;         // beyond the segment ends
            rec.t = t; rec.p = P;
            Vec3 outward = normalize((P - p0) - m * axis);
            rec.set_face_normal(r, outward);
            rec.u = 0; rec.v = 0; rec.mat = mat;
            return true;
        }
        return false;
    }
    bool bounding_box(AABB& out) const override {
        Vec3 e(radius, radius, radius);
        Vec3 mn(std::fmin(p0.x,p1.x), std::fmin(p0.y,p1.y), std::fmin(p0.z,p1.z));
        Vec3 mx(std::fmax(p0.x,p1.x), std::fmax(p0.y,p1.y), std::fmax(p0.z,p1.z));
        out = AABB(mn - e, mx + e);
        return true;
    }
private:
    Vec3 axis; float length;
};
