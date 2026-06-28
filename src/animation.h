#pragma once
// ---------------------------------------------------------------------------
// animation.h — drive any value over time with keyframes + easing.
// ---------------------------------------------------------------------------
#include "vec3.h"
#include <vector>
#include <algorithm>
#include <cmath>

inline float ease_linear(float t)        { return t; }
inline float ease_in_out_cubic(float t)  { return t < 0.5f ? 4*t*t*t : 1 - std::pow(-2*t+2, 3)/2; }
inline float ease_in_out_quad(float t)   { return t < 0.5f ? 2*t*t   : 1 - std::pow(-2*t+2, 2)/2; }
inline float ease_out_cubic(float t)     { return 1 - std::pow(1 - t, 3); }
inline float ease_in_cubic(float t)      { return t*t*t; }

template <typename T>
struct Keyframe { float time; T value; };

template <typename T>
class Animated {
public:
    using EaseFn = float (*)(float);
    EaseFn easing = ease_in_out_cubic;

    void add(float time, const T& value) {
        keys.push_back({time, value});
        std::sort(keys.begin(), keys.end(),
                  [](const Keyframe<T>& a, const Keyframe<T>& b) { return a.time < b.time; });
    }

    T eval(float t) const {
        if (keys.empty()) return T{};
        if (t <= keys.front().time) return keys.front().value;
        if (t >= keys.back().time)  return keys.back().value;
        for (std::size_t i = 0; i + 1 < keys.size(); ++i) {
            if (t >= keys[i].time && t <= keys[i+1].time) {
                float span  = keys[i+1].time - keys[i].time;
                float local = (span > 0.0f) ? (t - keys[i].time) / span : 0.0f;
                float e     = easing ? easing(local) : local;
                return blend(keys[i].value, keys[i+1].value, e);
            }
        }
        return keys.back().value;
    }

private:
    std::vector<Keyframe<T>> keys;
    static T blend(const T& a, const T& b, float u) { return a + (b - a) * u; }
};

using AnimatedVec3  = Animated<Vec3>;
using AnimatedFloat = Animated<float>;
