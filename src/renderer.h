#pragma once
// ---------------------------------------------------------------------------
// renderer.h — the path tracer.
//
// ray_color() is the Monte-Carlo light-transport integrator: follow a ray as
// it bounces, multiplying in each surface's tint (attenuation) and adding any
// emission, until it escapes to the environment or Russian roulette kills it.
//
// render() shoots `samples_per_pixel` jittered rays per pixel, averages them,
// applies the ACES filmic tone map + gamma, and writes 8-bit RGB. The pixel
// loop is parallelized with OpenMP.
// ---------------------------------------------------------------------------
#include "hittable.h"
#include "material.h"
#include "camera.h"
#include "environment.h"

#include <vector>
#include <atomic>
#include <cstddef>
#include <cstdio>

struct RenderSettings {
    int   width             = 1280;
    int   height            = 720;
    int   samples_per_pixel = 64;
    int   max_depth         = 12;
    Color sky_top           = Color(0.50f, 0.70f, 1.00f); // gradient fallback when no HDR
    const Environment* env  = nullptr;                    // if set, used instead of gradient
};

// ACES filmic tone-mapping curve — maps unbounded HDR radiance into [0,1]
// with film-like highlight roll-off.
inline Color aces_tonemap(const Color& x) {
    auto curve = [](float v) {
        return clampf((v * (2.51f * v + 0.03f)) / (v * (2.43f * v + 0.26f) + 0.59f), 0.0f, 1.0f);
    };
    return {curve(x.x), curve(x.y), curve(x.z)};
}

inline Color ray_color(const Ray& r, const Hittable& world,
                       const RenderSettings& cfg, int depth) {
    if (depth <= 0) return Color(0, 0, 0);  // bounce budget exhausted

    HitRecord rec;
    // t_min = 0.001 ignores self-intersections ("shadow acne").
    if (!world.hit(r, 0.001f, INF, rec)) {
        // Missed all geometry → sample the environment.
        if (cfg.env && cfg.env->valid())
            return cfg.env->sample(r.dir);
        // Fallback: vertical sky gradient.
        Vec3  unit = normalize(r.dir);
        float a    = 0.5f * (unit.y + 1.0f);
        return (1.0f - a) * Color(1, 1, 1) + a * cfg.sky_top;
    }

    Color emitted = rec.mat->emitted(rec.u, rec.v, rec.p);

    Ray   scattered;
    Color attenuation;
    if (!rec.mat->scatter(r, rec, attenuation, scattered))
        return emitted;  // light source or fully absorbed

    // Russian roulette after the first few bounces: randomly terminate dim
    // paths and boost survivors so the result stays unbiased.
    int bounce = cfg.max_depth - depth;
    if (bounce > 4) {
        float p = clampf(std::fmax(attenuation.x, std::fmax(attenuation.y, attenuation.z)),
                         0.05f, 1.0f);
        if (random_float() > p) return emitted;
        attenuation = attenuation / p;
    }

    return emitted + attenuation * ray_color(scattered, world, cfg, depth - 1);
}

// Renders into `pixels` as tightly-packed 8-bit RGB, row 0 = top of image.
inline void render(const Hittable& world, const Camera& cam,
                   const RenderSettings& cfg, std::vector<unsigned char>& pixels) {
    pixels.assign(static_cast<std::size_t>(cfg.width) * cfg.height * 3, 0);
    std::atomic<int> rows_done{0};

    #pragma omp parallel for schedule(dynamic, 1)
    for (int j = 0; j < cfg.height; ++j) {
        for (int i = 0; i < cfg.width; ++i) {
            Color col(0, 0, 0);
            for (int s = 0; s < cfg.samples_per_pixel; ++s) {
                float u = (i + random_float()) / (cfg.width  - 1);
                float v = (j + random_float()) / (cfg.height - 1);
                col += ray_color(cam.generate_ray(u, v), world, cfg, cfg.max_depth);
            }
            col = col / static_cast<float>(cfg.samples_per_pixel);
            col = gamma_correct(aces_tonemap(col));   // tone map, then gamma

            // Flip vertically: screen v=0 is the bottom, PNG row 0 is the top.
            std::size_t idx = (static_cast<std::size_t>(cfg.height - 1 - j) * cfg.width + i) * 3;
            pixels[idx + 0] = static_cast<unsigned char>(255.99f * clampf(col.x, 0.0f, 1.0f));
            pixels[idx + 1] = static_cast<unsigned char>(255.99f * clampf(col.y, 0.0f, 1.0f));
            pixels[idx + 2] = static_cast<unsigned char>(255.99f * clampf(col.z, 0.0f, 1.0f));
        }
        int d = ++rows_done;
        if (d % 16 == 0 || d == cfg.height)
            std::fprintf(stderr, "\rRendering: %4d / %d rows", d, cfg.height);
    }
    std::fprintf(stderr, "\n");
}
