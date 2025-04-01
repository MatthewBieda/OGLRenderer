# 3D Rendering Engine Development Roadmap

## Version 1
- Phong Lighting (Phong & Blinn-Phong)
- 3D Model Loading (Supporting Diffuse & Specular Maps)
- Free Camera
- UI Mode
- Directional Light (Sunlight)
- Point Lights (With proper Attenuation)
- Spotlights (Implemented as a flashlight)
- Wireframe Mode
- All parameters dynamically adjustable in the application

[![OGLRenderer v1.5](https://img.youtube.com/vi/pXJLGCEeVWc/0.jpg)](https://www.youtube.com/watch?v=pXJLGCEeVWc)

## Version 1.5
- Skyboxes
- Dynamic PCF Shadow Maps
- Anti-Aliasing (MSAA)
- Controller Support

[![OGLRenderer v1.5](https://img.youtube.com/vi/YpSUtLvGxqE/0.jpg)](https://www.youtube.com/watch?v=YpSUtLvGxqE)

## Version 2
- Physically Based Materials and Lighting (Cook-Torrance)
- Normal / Bump / Parallax Mapping
- HDR/Tone Mapping/Gamma Correction
- Deferred Rendering
- Cascading Shadow Maps
- Bloom/SSAO/Reflections
- Image Based Lighting
- Instanced Rendering
- Skeletal Animations

## Version 3
- Vulkan Rewrite
- Hardware Accelerated Ray Tracing
- DLSS
- Temporal Anti-Aliasing
- Per-Object Motion Blur
- Frustrum Culling / Occlusion Culling
- Volumetrics
- Caustics
- Spherical Harmonics

# Build Instructions

- Install the latest CMake version from the official website
- Run the following from the command line in project root directory
- cmake -S . -B build -DCMAKE_POLICY_VERSION_MINIMUM=3.5
- cmake --build build --config Release
- Move the .exe file into root of project

# Usage Instructions

- Spacebar to switch between Free Camera and UI Mode. 
- Plug in an Xbox controller to navigate around the scene and jump with the A button (no collision yet).