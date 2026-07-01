#pragma once
// Bounding Volume Hierarchy: recursively splits objects so a ray only tests
// the few primitives near it instead of all of them. Essential for meshes.
#include "hittable.h"
#include "aabb.h"
#include <vector>
#include <algorithm>

class BVHNode : public Hittable {
public:
    BVHNode(std::vector<std::shared_ptr<Hittable>>& objs, size_t start, size_t end) {
        // Choose the axis with the widest spread of this subset.
        AABB bounds; bool first = true;
        for (size_t i = start; i < end; ++i) {
            AABB b; objs[i]->bounding_box(b);
            bounds = first ? b : surrounding_box(bounds, b);
            first = false;
        }
        Vec3 ext = bounds.mx - bounds.mn;
        int axis = (ext.x > ext.y && ext.x > ext.z) ? 0 : (ext.y > ext.z ? 1 : 2);
        auto cmp = [axis](const std::shared_ptr<Hittable>& a, const std::shared_ptr<Hittable>& b) {
            AABB ba, bb; a->bounding_box(ba); b->bounding_box(bb);
            return vcomp(ba.mn, axis) < vcomp(bb.mn, axis);
        };

        size_t span = end - start;
        if (span == 1) { left = right = objs[start]; }
        else if (span == 2) {
            left  = objs[start];
            right = objs[start + 1];
        } else {
            std::sort(objs.begin() + start, objs.begin() + end, cmp);
            size_t mid = start + span / 2;
            left  = std::make_shared<BVHNode>(objs, start, mid);
            right = std::make_shared<BVHNode>(objs, mid, end);
        }
        AABB bl, br; left->bounding_box(bl); right->bounding_box(br);
        box = surrounding_box(bl, br);
    }

    bool hit(const Ray& r, float t_min, float t_max, HitRecord& rec) const override {
        if (!box.hit(r, t_min, t_max)) return false;
        bool hit_left  = left->hit(r, t_min, t_max, rec);
        bool hit_right = right->hit(r, t_min, hit_left ? rec.t : t_max, rec);
        return hit_left || hit_right;
    }
    bool bounding_box(AABB& output_box) const override { output_box = box; return true; }

private:
    std::shared_ptr<Hittable> left, right;
    AABB box;
};
