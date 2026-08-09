C++ Path Tracer

A physically-based 3D renderer built from scratch in C++ (no engine, no graphics library) - simulates light rays for realistic reflections, refraction, and shadows.

## Build & Run

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
./build/raytracer
```

## Features
- Diffuse, metal, and glass materials
- BVH acceleration
- Depth of field, anti-aliasing, ACES tone mapping
- Multi-threaded (OpenMP)
