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

## Version 1.5
- Skyboxes
- Dynamic PCF Shadow Maps
- Anti-Aliasing (MSAA)
- Controller Support

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

No build system implemented yet, just open the .sln file with Visual Studio if you want to play around with it.

Spacebar to switch between Free Camera and UI Mode. 
Plug in an Xbox controller to navigate around the scene and jump with the A button (no collision yet)