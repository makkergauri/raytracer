#pragma once
#include "vec3.h"
#include "stb_image.h"
#include <string>
#include <cstddef>
#include <iostream>

class Environment {
public:
    float intensity = 1.0f;
    ~Environment() { if (data) stbi_image_free(data); }

    bool load(const std::string& path) {
        data = stbi_loadf(path.c_str(), &width, &height, &channels, 3);
        channels = 3;
        if (!data) { std::cerr << "Environment: could not load '" << path << "'\n"; return false; }
        return true;
    }
    bool valid() const { return data != nullptr; }

    Color sample(const Vec3& dir) const {
        Vec3 d = normalize(dir);
        float u = 0.5f + std::atan2(d.z, d.x) / (2.0f * PI);
        float v = 0.5f - std::asin(clampf(d.y, -1.0f, 1.0f)) / PI;
        float fx = u * (width - 1);
        float fy = v * (height - 1);
        int x0 = clampi((int)std::floor(fx), 0, width - 1);
        int y0 = clampi((int)std::floor(fy), 0, height - 1);
        int x1 = clampi(x0 + 1, 0, width - 1);
        int y1 = clampi(y0 + 1, 0, height - 1);
        float tx = fx - x0, ty = fy - y0;
        Color top = (1 - tx) * texel(x0, y0) + tx * texel(x1, y0);
        Color bot = (1 - tx) * texel(x0, y1) + tx * texel(x1, y1);
        return intensity * ((1 - ty) * top + ty * bot);
    }
private:
    float* data = nullptr;
    int width = 0, height = 0, channels = 0;
    static int clampi(int x, int lo, int hi) { return x < lo ? lo : (x > hi ? hi : x); }
    Color texel(int x, int y) const {
        const float* p = data + (static_cast<std::size_t>(y) * width + x) * 3;
        return Color(p[0], p[1], p[2]);
    }
};
echo "environment.h written"