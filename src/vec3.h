#pragma once
// ---------------------------------------------------------------------------
// vec3.h — 3D vector math, the foundation everything else is built on.
//   * Vec3 doubles as a point, a direction, AND an RGB color (Color alias).
//   * Free functions for dot/cross/normalize/reflect/refract.
//   * Thread-safe RNG + sampling helpers (used by the path tracer).
// ---------------------------------------------------------------------------
#include <cmath>
#include <random>
#include <limits>
#include <iostream>

constexpr float PI       = 3.14159265358979323846f;
constexpr float INF      = std::numeric_limits<float>::infinity();
constexpr float DEG2RAD  = PI / 180.0f;

struct Vec3 {
    float x, y, z;

    Vec3() : x(0), y(0), z(0) {}
    explicit Vec3(float v) : x(v), y(v), z(v) {}   // explicit: no silent scalar->vector
    Vec3(float x_, float y_, float z_) : x(x_), y(y_), z(z_) {}

    Vec3 operator-() const { return {-x, -y, -z}; }

    Vec3& operator+=(const Vec3& o) { x += o.x; y += o.y; z += o.z; return *this; }
    Vec3& operator-=(const Vec3& o) { x -= o.x; y -= o.y; z -= o.z; return *this; }
    Vec3& operator*=(float t)       { x *= t;   y *= t;   z *= t;   return *this; }
    Vec3& operator/=(float t)       { return *this *= (1.0f / t); }

    float length()         const { return std::sqrt(length_squared()); }
    float length_squared() const { return x * x + y * y + z * z; }

    // True if the vector is very close to zero in all dimensions (degenerate scatter).
    bool near_zero() const {
        const float s = 1e-8f;
        return std::fabs(x) < s && std::fabs(y) < s && std::fabs(z) < s;
    }
};

// Semantic aliases — same struct, clearer intent at call sites.
using Point3 = Vec3;
using Color  = Vec3;

// ---- arithmetic ------------------------------------------------------------
inline Vec3 operator+(const Vec3& a, const Vec3& b) { return {a.x + b.x, a.y + b.y, a.z + b.z}; }
inline Vec3 operator-(const Vec3& a, const Vec3& b) { return {a.x - b.x, a.y - b.y, a.z - b.z}; }
inline Vec3 operator*(const Vec3& a, const Vec3& b) { return {a.x * b.x, a.y * b.y, a.z * b.z}; } // component-wise (e.g. color * albedo)
inline Vec3 operator*(float t, const Vec3& v)       { return {t * v.x, t * v.y, t * v.z}; }
inline Vec3 operator*(const Vec3& v, float t)       { return t * v; }
inline Vec3 operator/(const Vec3& v, float t)       { return (1.0f / t) * v; }

// ---- products / normalization ---------------------------------------------
inline float dot(const Vec3& a, const Vec3& b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
inline Vec3  cross(const Vec3& a, const Vec3& b) {
    return {a.y * b.z - a.z * b.y,
            a.z * b.x - a.x * b.z,
            a.x * b.y - a.y * b.x};
}
inline Vec3 normalize(const Vec3& v) { return v / v.length(); }

// ---- optics ----------------------------------------------------------------
inline Vec3 reflect(const Vec3& v, const Vec3& n) {
    return v - 2.0f * dot(v, n) * n;
}
// Snell's law refraction; etai_over_etat = n_incident / n_transmitted.
inline Vec3 refract(const Vec3& uv, const Vec3& n, float etai_over_etat) {
    float cos_theta = std::fmin(dot(-uv, n), 1.0f);
    Vec3 r_out_perp     = etai_over_etat * (uv + cos_theta * n);
    Vec3 r_out_parallel = -std::sqrt(std::fabs(1.0f - r_out_perp.length_squared())) * n;
    return r_out_perp + r_out_parallel;
}

// ---- random / sampling -----------------------------------------------------
// One generator PER THREAD so OpenMP workers never contend or race.
inline float random_float() {
    static thread_local std::mt19937 gen(std::random_device{}());
    static thread_local std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    return dist(gen);
}
inline float random_float(float lo, float hi)  { return lo + (hi - lo) * random_float(); }
inline Vec3  random_vec()                       { return {random_float(), random_float(), random_float()}; }
inline Vec3  random_vec(float lo, float hi)     { return {random_float(lo, hi), random_float(lo, hi), random_float(lo, hi)}; }

inline Vec3 random_in_unit_sphere() {
    while (true) {
        Vec3 p = random_vec(-1.0f, 1.0f);
        if (p.length_squared() < 1.0f) return p;
    }
}
inline Vec3 random_unit_vector() { return normalize(random_in_unit_sphere()); }

inline Vec3 random_in_hemisphere(const Vec3& normal) {
    Vec3 p = random_in_unit_sphere();
    return (dot(p, normal) > 0.0f) ? p : -p;
}
// Disk sampling — used by the camera lens for depth of field.
inline Vec3 random_in_unit_disk() {
    while (true) {
        Vec3 p(random_float(-1.0f, 1.0f), random_float(-1.0f, 1.0f), 0.0f);
        if (p.length_squared() < 1.0f) return p;
    }
}

// ---- utilities -------------------------------------------------------------
inline float clampf(float x, float lo, float hi) { return x < lo ? lo : (x > hi ? hi : x); }
inline Vec3  gamma_correct(const Vec3& c, float g = 2.2f) {
    float inv = 1.0f / g;
    return {std::pow(c.x, inv), std::pow(c.y, inv), std::pow(c.z, inv)};
}
