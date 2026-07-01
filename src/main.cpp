// ---------------------------------------------------------------------------
// main.cpp — "The Bridge Between Worlds" — Milestone 1: bridge geometry.
//   Lit with the studio HDR for now; night sky + lanterns come next.
// ---------------------------------------------------------------------------
#include <iostream>
#include <vector>
#include <cmath>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#undef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#undef STB_IMAGE_WRITE_IMPLEMENTATION

#include "vec3.h"
#include "camera.h"
#include "sphere.h"
#include "shapes.h"
#include "material.h"
#include "texture.h"
#include "bvh.h"
#include "environment.h"
#include "renderer.h"

using Obj = std::vector<std::shared_ptr<Hittable>>;

// Stone with slight per-instance color variation for texture variety.
static std::shared_ptr<Material> stone() {
    float n = random_float(-0.05f, 0.05f);
    return std::make_shared<Lambertian>(Color(0.36f + n, 0.31f + n, 0.26f + n));
}

// Procedural stone bridge: deck + railings + posts + arches + abutments.
static void buildBridge(Obj& w) {
    const float halfW = 1.2f, z0 = -12.0f, z1 = 12.0f, deckTop = 0.2f;

    // Deck slab, split into segments so the stone color varies along it.
    const int segs = 12;
    for (int i = 0; i < segs; ++i) {
        float za = z0 + (z1 - z0) * i / segs, zb = z0 + (z1 - z0) * (i + 1) / segs;
        w.push_back(std::make_shared<Box>(Vec3(-halfW, 0.0f, za), Vec3(halfW, deckTop, zb), stone()));
    }
    // Side railing walls.
    for (int s = -1; s <= 1; s += 2) {
        float xc = s * 1.125f;
        w.push_back(std::make_shared<Box>(Vec3(xc - 0.075f, deckTop, z0),
                                          Vec3(xc + 0.075f, deckTop + 0.35f, z1), stone()));
    }
    // Railing posts.
    for (float z = z0 + 0.8f; z < z1; z += 1.7f)
        for (int s = -1; s <= 1; s += 2) {
            float xc = s * 1.125f;
            w.push_back(std::make_shared<Box>(Vec3(xc - 0.09f, deckTop, z - 0.09f),
                                              Vec3(xc + 0.09f, deckTop + 0.55f, z + 0.09f), stone()));
        }
    // Two underside arches, each 8 cylinder segments forming a downward curve.
    const int A = 8; const float archSpan = 6.0f, archH = 2.0f, ar = 0.14f;
    for (int s = -1; s <= 1; s += 2) {
        float xa = s * 1.0f;
        Point3 prev;
        for (int k = 0; k <= A; ++k) {
            float phi = PI * k / A;
            Point3 p(xa, -archH * std::sin(phi), -archSpan * std::cos(phi));
            if (k > 0) w.push_back(std::make_shared<Cylinder>(prev, p, ar, stone()));
            prev = p;
        }
    }
    // Abutment blocks at the banks + small central piers.
    w.push_back(std::make_shared<Box>(Vec3(-halfW, -2.6f, z0), Vec3(halfW, 0.0f, z0 + 1.4f), stone()));
    w.push_back(std::make_shared<Box>(Vec3(-halfW, -2.6f, z1 - 1.4f), Vec3(halfW, 0.0f, z1), stone()));
    for (int s = -1; s <= 1; s += 2) {
        float xa = s * 1.0f;
        w.push_back(std::make_shared<Cylinder>(Point3(xa, -2.4f, 0), Point3(xa, -2.0f, 0), 0.18f, stone()));
    }
}

int main() {
    RenderSettings cfg;
    cfg.width = 1280; cfg.height = 720;
    cfg.samples_per_pixel = 64;   // lower for faster previews
    cfg.max_depth = 16;

    Environment env;
    if (env.load("assets/hdri/studio.hdr")) { env.intensity = 1.0f; cfg.env = &env; }
    else std::cerr << "No HDRI - gradient sky.\n";

    Obj w;
    // Water / ground plane.
    w.push_back(std::make_shared<Quad>(Point3(-30, -2.6f, -30), Vec3(60, 0, 0), Vec3(0, 0, 60),
                std::make_shared<Lambertian>(Color(0.18f, 0.20f, 0.22f))));
    buildBridge(w);

    auto bvh = std::make_shared<BVHNode>(w, 0, w.size());
    std::cerr << "Scene objects: " << w.size() << "\n";

    float aspect = static_cast<float>(cfg.width) / cfg.height;
    Point3 from(-6.5f, 2.8f, 9.0f), look(0, 0.3f, 0);
    Camera cam(from, look, Vec3(0, 1, 0), 42.0f, aspect, 0.0f, (from - look).length());

    std::vector<unsigned char> px;
    render(*bvh, cam, cfg, px);
    stbi_write_png("output/bridge.png", cfg.width, cfg.height, 3, px.data(), cfg.width * 3);
    std::cout << "Wrote output/bridge.png\n";
    return 0;
}
