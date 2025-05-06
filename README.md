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

[![OGLRenderer v1.0](https://img.youtube.com/vi/pXJLGCEeVWc/0.jpg)](https://www.youtube.com/watch?v=pXJLGCEeVWc)

## Version 1.5
- Skyboxes
- Dynamic PCF Shadow Maps
- Anti-Aliasing (MSAA)
- Controller Support

[![OGLRenderer v1.5](https://img.youtube.com/vi/YpSUtLvGxqE/0.jpg)](https://www.youtube.com/watch?v=YpSUtLvGxqE)

## Version 1.6
- Support arbitrary model loading/unloading
- Adopt CMake for a proper build system
- Move to ImGui Docking to improve user experience

[![OGLRenderer v1.6](https://img.youtube.com/vi/e7xCygjkNcU/0.jpg)](https://www.youtube.com/watch?v=e7xCygjkNcU)

## Version 1.7
- Instanced Rendering
- Normal Mapping
- Various Resolution modes
- Crytek Sponza

[![OGLRenderer v1.7](https://img.youtube.com/vi/PC8fdYX1EY8/0.jpg)](https://www.youtube.com/watch?v=PC8fdYX1EY8)

## Version 2
- Physically Based Materials and Lighting (Cook-Torrance)
- HDR/Tone Mapping/Gamma Correction
- Image Based Lighting

I used PolyHaven to get the HDRI, CMFTStudio to bake the diffuse irradiance and prefilter maps in linear space, and IBLBaker to get the Smith BRDF LUT. Also used HDRI-to-CubeMap web utility to make the skybox. Loaded everything in as HDR with stb_image, except the LUT is just a PNG, then tonemapped and gamma corrected at the end. Made sure to parse Albedo and Emissive as sRGB also (hardware applies linear space transformation).

- Deferred Rendering
- Cascading Shadow Maps
- Bloom/SSAO/Reflections
- Skeletal Animations
- Spotlight & Point Light Shadows (Omnidirectional)
- Transparency

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

Builds tested on Arch Linux (GNU) and Windows 11 (MSVC)

# Usage Instructions

- Spacebar to switch between Free Camera and UI Mode. 
- Plug in an Xbox controller to navigate around the scene and jump with the A button (no collision yet).