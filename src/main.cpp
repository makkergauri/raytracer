#include <iostream>
#include <vector>
#include <chrono>

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
#include "renderer.h"

int main() {
    RenderSettings cfg;
    cfg.width             = 1280;
    cfg.height            = 720;
    cfg.samples_per_pixel = 128;
    cfg.max_depth         = 20;

    Environment env;
    const char* hdri_path = "assets/hdri/studio.hdr";
    if (env.load(hdri_path)) {
        env.intensity = 1.0f;
        cfg.env = &env;
        std::cerr << "Loaded HDRI: " << hdri_path << "\n";
    } else {
        std::cerr << "No HDRI found at " << hdri_path << " - using gradient sky.\n";
    }

    HittableList world;
    auto checker = std::make_shared<CheckerTexture>(Color(0.18f, 0.28f, 0.30f),
                                                    Color(0.88f, 0.88f, 0.90f), 3.0f);
    auto ground = std::make_shared<Lambertian>(checker);
    auto red    = std::make_shared<Lambertian>(Color(0.85f, 0.30f, 0.28f));
    auto chrome = std::make_shared<Metal>(Color(0.80f, 0.80f, 0.85f), 0.02f);
    auto glass  = std::make_shared<Dielectric>(1.5f);
    world.add(std::make_shared<Sphere>(Point3( 0.0f, -100.5f, -1.0f), 100.0f, ground));
    world.add(std::make_shared<Sphere>(Point3( 0.0f,    0.0f, -1.0f),   0.5f, red));
    world.add(std::make_shared<Sphere>(Point3( 1.0f,    0.0f, -1.0f),   0.5f, chrome));
    world.add(std::make_shared<Sphere>(Point3(-1.0f,    0.0f, -1.0f),   0.5f, glass));

    float aspect = static_cast<float>(cfg.width) / cfg.height;
    Camera cam(Point3(0.0f, 0.5f, 3.0f), Point3(0.0f, 0.0f, -1.0f),
               Vec3(0.0f, 1.0f, 0.0f), 40.0f, aspect);

    std::vector<unsigned char> pixels;
    auto t0 = std::chrono::high_resolution_clock::now();
    render(world, cam, cfg, pixels);
    auto t1 = std::chrono::high_resolution_clock::now();
    std::cerr << "Rendered " << cfg.width << "x" << cfg.height << " @ "
              << cfg.samples_per_pixel << " spp in "
              << std::chrono::duration<double>(t1 - t0).count() << " s\n";

    const char* path = "output/frames/frame_0000.png";
    if (!stbi_write_png(path, cfg.width, cfg.height, 3, pixels.data(), cfg.width * 3)) {
        std::cerr << "ERROR: failed to write " << path << "\n";
        return 1;
    }
    std::cout << "Wrote " << path << "\n";
    return 0;
}
echo "main.cpp written"