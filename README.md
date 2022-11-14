# cpu-renderer

A simple CPU renderer, supporting congurable cameras, objects, lights and effects.

## Dependencies

- OpenMP
- OpenCV
- Eigen
- Intel MKL

## Running

```
cpu-renderer [config_path]
```

- `config_path`: Optional.

  Default: `../example/default-config.yaml`

## Configuration

Example congiurations are in the directory `example/`.

In YAML files, a vector in inputted as a parameter in the format of `[x, y, z]`.

- `objects`: Accept an array of objects.

  - `path`: String. **Only accept OBJ files.**

  - `base-path`: String. The base path of assets (e.g. textures).

  - `pos`: 3D vector.

  - `rotation`: 3D vector.

  - `scale`: 3D vector.

  - `shading-type`: Optional. String.

    Default: `default`

    Supported options:

    - `default`: Bilnn-phong shading.

    - `cel`: Cel shading.

    - `pbr`: PBR.

- `lights`: Accept an array of lights.

  - `pos`: 3D vector.

  - `color`: 3D vector, with components in the order of RGB.

  - `intensity`: Float.

- `camera`: Camera settings.

  - `pos`: 3D vector.

  - `rotation`: 3D vector.

  - `fov`: Float. Vertical FOV.

  - `near-plane`: Float.

  - `far-plane`: Float.

  - `width`: Integer. Horizontal resolution.

  - `height`: Integer. Vertical resolution.

- `threads-num`: Integer.

- `background-color`: 3D vector, with components in the order of RGB.

- `rimlight`: Rimlight, usually used with cel shading.

  - `enable`: Boolean.

- `bloom`:

  - `enable`: Boolean.

  - `strength`: Float.

  - `radius`: Float.

  - `iteration`: Integer.

### MTL files

This program supports MTL files following this standard `http://exocortex.com/blog/extending_wavefront_mtl_to_support_pbr` supporting PBR.

**Attention! For convenience, some interfaces do NOT follow from the normal standard:**

- The bump map (`bump`) is used as the AO map.

## Development

### Class relationship

```
Scene +- Object -- Shape +- Triangle -- Vertex
                         +- Material -- (Mipmap) -- Texture
      +- Light
      +- Camera
Buffer -- Texture
```

### Coordinate system

This program uses right-handed coordinate system.

For camera:

- X: Left
- Y: Up
- Z: Look At

### Rendering pipeline

The rendering pipeline is defined in the function `render()` in `src/main.cpp`.

### MSAA

In the file `src/include/msaa.hpp`:

- `static const size_t msaa::MSAA_LEVEL` defines the number of sampling points.

- `static const vec2 msaa::samples_coord_delta[MSAA_LEVEL]` defines the relative position of sampling points.

### Mipmap

In the file `src/include/texture/mipmap.hpp`:

- `const int mipmap::MIPMAP_LEVEL` defines the number of mipmap levels.

### Outline

In the file `src/include/outline.hpp`:

- `const float outline::OUTLINE_WIDTH` defines the outline width.

- `const vec3 outline::OUTLINE_COLOR` defines the outline color.

### Rimlight

In the file `src/include/rimlight.hpp`:

- `const std::vector<std::tuple<size_t, size_t, float>> rimlight::RIMLIGHT_DELTA` defines the sampling offsets.