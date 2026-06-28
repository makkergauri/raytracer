#pragma once
// ---------------------------------------------------------------------------
// texture.h — surface color that can vary across a surface.
//
//   value(u, v, p) returns the color at texture coords (u,v) / world point p.
//   SolidColor     : one flat color (what a plain albedo used to be).
//   CheckerTexture : 3D checkerboard — flips color every unit cell in space,
//                    so it tiles cleanly across the ground without UV seams.
// ---------------------------------------------------------------------------
#include "vec3.h"
#include <memory>
#include <cmath>

class Texture {
public:
    virtual ~Texture() = default;
    virtual Color value(float u, float v, const Point3& p) const = 0;
};

class SolidColor : public Texture {
public:
    explicit SolidColor(const Color& c) : color(c) {}
    Color value(float, float, const Point3&) const override { return color; }
private:
    Color color;
};

class CheckerTexture : public Texture {
public:
    // `scale` = checks per world unit (bigger = smaller squares).
    CheckerTexture(std::shared_ptr<Texture> even_tex,
                   std::shared_ptr<Texture> odd_tex, float scale = 1.0f)
        : even(std::move(even_tex)), odd(std::move(odd_tex)), inv_scale(scale) {}

    CheckerTexture(const Color& c1, const Color& c2, float scale = 1.0f)
        : even(std::make_shared<SolidColor>(c1)),
          odd(std::make_shared<SolidColor>(c2)),
          inv_scale(scale) {}

    Color value(float u, float v, const Point3& p) const override {
        int xi = static_cast<int>(std::floor(inv_scale * p.x));
        int yi = static_cast<int>(std::floor(inv_scale * p.y));
        int zi = static_cast<int>(std::floor(inv_scale * p.z));
        bool is_even = ((xi + yi + zi) & 1) == 0;
        return is_even ? even->value(u, v, p) : odd->value(u, v, p);
    }

private:
    std::shared_ptr<Texture> even, odd;
    float inv_scale;
};