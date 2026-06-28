// ---------------------------------------------------------------------------
// main.cpp — Animated short: orbiting camera over a ring of spheres.
//   Renders an image sequence (output/frames/frame_XXXX.png) that ffmpeg
//   stitches into a video. Camera path + depth of field are keyframed.
// ---------------------------------------------------------------------------
#include <iostream>
#include <vector>
#include <chrono>
#include <cstdio>
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
#include "material.h"
#include "texture.h"
#include "environment.h"
#include "animation.h"
#include "renderer.h"

int main() {
    // ---- Film settings -----------------------------------------------------
    const int   WIDTH   = 1280;
    const int   HEIGHT  = 720;
    const int   SPP     = 64;
    const int   DEPTH   = 16;
    const int   FPS     = 24;
    const float DURATION= 4.0f;                 // seconds
    const int   FRAMES  = int(FPS * DURATION);  // 96

    RenderSettings cfg;
    cfg.width = WIDTH; cfg.height = HEIGHT;
    cfg.samples_per_pixel = SPP; cfg.max_depth = DEPTH;

    // ---- Environment -------------------------------------------------------
    Environment env;
    if (env.load("assets/hdri/studio.hdr")) { env.intensity = 1.0f; cfg.env = &env; }
    else std::cerr << "No HDRI - using gradient sky.\n";

    // ---- Scene: glass hero + a ring of colored / metal / glass spheres -----
    HittableList world;
    auto checker = std::make_shared<CheckerTexture>(Color(0.16f,0.20f,0.22f),
                                                    Color(0.86f,0.87f,0.90f), 2.2f);
    world.add(std::make_shared<Sphere>(Point3(0,-100.5f,0), 100.0f,
              std::make_shared<Lambertian>(checker)));

    // Hero glass sphere at the centre (rests on the floor).
    world.add(std::make_shared<Sphere>(Point3(0,0.0f,0), 0.5f,
              std::make_shared<Dielectric>(1.5f)));

    // Palette of materials to cycle through around the ring.
    std::vector<std::shared_ptr<Material>> palette = {
        std::make_shared<Lambertian>(Color(0.75f,0.10f,0.15f)),  // ruby
        std::make_shared<Metal>(Color(0.95f,0.78f,0.38f), 0.04f),// gold
        std::make_shared<Lambertian>(Color(0.10f,0.45f,0.22f)),  // emerald
        std::make_shared<Dielectric>(1.5f),                      // glass
        std::make_shared<Lambertian>(Color(0.12f,0.22f,0.62f)),  // sapphire
        std::make_shared<Metal>(Color(0.91f,0.92f,0.95f), 0.02f),// silver
        std::make_shared<Lambertian>(Color(0.55f,0.22f,0.62f)),  // amethyst
        std::make_shared<Metal>(Color(0.82f,0.45f,0.30f), 0.08f),// copper
    };

    const int   RING = 16;
    const float Rr   = 1.35f, sr = 0.22f, sy = -0.5f + 0.22f;
    for (int i = 0; i < RING; ++i) {
        float a = 2.0f * PI * i / RING;
        Point3 p(Rr*std::cos(a), sy, Rr*std::sin(a));
        world.add(std::make_shared<Sphere>(p, sr, palette[i % palette.size()]));
    }
    // A few larger accent spheres for depth.
    world.add(std::make_shared<Sphere>(Point3(-0.95f,-0.20f, 0.55f), 0.30f, palette[1]));
    world.add(std::make_shared<Sphere>(Point3( 0.90f,-0.20f,-0.60f), 0.30f, palette[5]));
    world.add(std::make_shared<Sphere>(Point3( 0.20f,-0.27f, 1.05f), 0.23f, palette[0]));

    // ---- Keyframed camera (orbit + a gentle height dip) --------------------
    Point3 look(0.0f, 0.05f, 0.0f);
    AnimatedFloat angle;  angle.add(0.0f, -0.6f);  angle.add(DURATION,  1.9f); // radians swept
    AnimatedFloat height; height.add(0.0f, 0.95f); height.add(DURATION*0.5f, 0.45f); height.add(DURATION, 0.85f);
    AnimatedFloat radius; radius.add(0.0f, 3.4f);  radius.add(DURATION, 2.9f);        // slow push-in
    const float vfov = 38.0f, aperture = 0.05f;
    float aspect = float(WIDTH) / HEIGHT;

    std::cerr << "Rendering " << FRAMES << " frames @ " << WIDTH << "x" << HEIGHT
              << ", " << SPP << " spp...\n";
    auto t_all = std::chrono::high_resolution_clock::now();
    std::vector<unsigned char> pixels;

    for (int f = 0; f < FRAMES; ++f) {
        float t   = f / float(FPS);
        float ang = angle.eval(t), h = height.eval(t), rad = radius.eval(t);
        Point3 cam_pos(rad*std::cos(ang), h, rad*std::sin(ang));
        float focus = (cam_pos - Point3(0,0,0)).length();   // keep the hero sharp
        Camera cam(cam_pos, look, Vec3(0,1,0), vfov, aspect, aperture, focus);

        auto t0 = std::chrono::high_resolution_clock::now();
        render(world, cam, cfg, pixels);
        auto t1 = std::chrono::high_resolution_clock::now();

        char path[256];
        std::snprintf(path, sizeof(path), "output/frames/frame_%04d.png", f);
        stbi_write_png(path, WIDTH, HEIGHT, 3, pixels.data(), WIDTH*3);
        std::fprintf(stderr, "  frame %3d/%d  (%.1fs)\n", f+1, FRAMES,
                     std::chrono::duration<double>(t1-t0).count());
    }
    double total = std::chrono::duration<double>(
        std::chrono::high_resolution_clock::now() - t_all).count();
    std::cerr << "Done: " << FRAMES << " frames in " << total << " s\n";
    std::cout << "Wrote output/frames/frame_0000.png ... frame_"
              << (FRAMES-1) << ".png\n";
    return 0;
}
