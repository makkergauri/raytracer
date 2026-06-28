#pragma once
// ---------------------------------------------------------------------------
// material.h — how light interacts with a surface.
//
// Each material answers two questions for a ray that just hit it:
//   emitted() : does this surface give off light? (lights use this)
//   scatter() : where does the ray bounce next, and how is it tinted?
//               returns false if the ray is absorbed / shouldn't continue.
// ---------------------------------------------------------------------------
#include "hittable.h"
#include "texture.h"

class Material {
public:
    virtual ~Material() = default;

    virtual bool scatter(const Ray& /*r_in*/, const HitRecord& /*rec*/,
                         Color& /*attenuation*/, Ray& /*scattered*/) const {
        return false;  // default: absorbs everything
    }
    virtual Color emitted(float /*u*/, float /*v*/, const Point3& /*p*/) const {
        return Color(0, 0, 0);  // default: emits nothing
    }
};

// --- Lambertian (matte diffuse) --------------------------------------------
// Scatters toward a random direction biased around the normal; tints by albedo,
// which is now a Texture (a flat Color still works via the Color constructor).
class Lambertian : public Material {
public:
    std::shared_ptr<Texture> albedo;

    explicit Lambertian(const Color& a) : albedo(std::make_shared<SolidColor>(a)) {}
    explicit Lambertian(std::shared_ptr<Texture> tex) : albedo(std::move(tex)) {}

    bool scatter(const Ray& r_in, const HitRecord& rec,
                 Color& attenuation, Ray& scattered) const override {
        Vec3 scatter_dir = rec.normal + random_unit_vector();
        if (scatter_dir.near_zero()) scatter_dir = rec.normal;  // guard degenerate dir
        scattered   = Ray(rec.p, scatter_dir, r_in.time);
        attenuation = albedo->value(rec.u, rec.v, rec.p);
        return true;
    }
};

// --- Metal (reflective) -----------------------------------------------------
// Mirror reflection, perturbed by `fuzz` for a brushed/glossy look.
class Metal : public Material {
public:
    Color albedo;
    float fuzz;
    Metal(const Color& a, float f) : albedo(a), fuzz(f < 1.0f ? f : 1.0f) {}

    bool scatter(const Ray& r_in, const HitRecord& rec,
                 Color& attenuation, Ray& scattered) const override {
        Vec3 reflected = reflect(normalize(r_in.dir), rec.normal);
        scattered   = Ray(rec.p, reflected + fuzz * random_in_unit_sphere(), r_in.time);
        attenuation = albedo;
        return dot(scattered.dir, rec.normal) > 0.0f;  // kill rays that fuzz below surface
    }
};

// --- Dielectric (glass / water) --------------------------------------------
// Refracts via Snell's law, or reflects when refraction is impossible
// (total internal reflection) or when the Fresnel term says so (Schlick).
class Dielectric : public Material {
public:
    float ir;  // index of refraction (1.5 ≈ glass, 1.33 ≈ water)
    explicit Dielectric(float index_of_refraction) : ir(index_of_refraction) {}

    bool scatter(const Ray& r_in, const HitRecord& rec,
                 Color& attenuation, Ray& scattered) const override {
        attenuation = Color(1, 1, 1);  // clear glass absorbs nothing
        float refraction_ratio = rec.front_face ? (1.0f / ir) : ir;

        Vec3  unit_dir  = normalize(r_in.dir);
        float cos_theta = std::fmin(dot(-unit_dir, rec.normal), 1.0f);
        float sin_theta = std::sqrt(1.0f - cos_theta * cos_theta);

        bool cannot_refract = refraction_ratio * sin_theta > 1.0f;
        Vec3 direction;
        if (cannot_refract || reflectance(cos_theta, refraction_ratio) > random_float())
            direction = reflect(unit_dir, rec.normal);
        else
            direction = refract(unit_dir, rec.normal, refraction_ratio);

        scattered = Ray(rec.p, direction, r_in.time);
        return true;
    }

private:
    // Schlick's polynomial approximation of the Fresnel reflectance.
    static float reflectance(float cosine, float ref_idx) {
        float r0 = (1.0f - ref_idx) / (1.0f + ref_idx);
        r0 = r0 * r0;
        return r0 + (1.0f - r0) * std::pow(1.0f - cosine, 5.0f);
    }
};

// --- DiffuseLight (emissive) ------------------------------------------------
// Emits a constant color and never scatters — this is how area lights work.
class DiffuseLight : public Material {
public:
    Color emit;
    explicit DiffuseLight(const Color& c) : emit(c) {}

    Color emitted(float /*u*/, float /*v*/, const Point3& /*p*/) const override {
        return emit;
    }
};