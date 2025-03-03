A C++ 3D rendering engine based largely on LearnOpenGL that implements all basic 3D graphics techniques and whatever advanced features I want to implement.

History of lightning models, I plan to implement the ability to dynamically change this in a scene to visually illustrate the differences. 

1. Flat Shading (1970s)
- Concept:
The simplest shading model.
Each polygon (triangle) is assigned a single color based on its normal and light source.
No interpolation between vertices.
- Pros:
Very fast and computationally cheap.
Good for low-polygon models or stylized art.
- Cons:
Hard edges between polygons make the object look faceted.
No smooth transitions between light and shadow.
- Use Cases:
Early 3D games.
Low-poly aesthetics.

2. Gouraud Shading (1971 - Henri Gouraud)
- Concept:
Interpolates lighting across vertices using linear interpolation.
Calculates lighting only at each vertex, then interpolates colors across the triangle.
- Pros:
Smoother appearance compared to flat shading.
Still relatively fast.
- Cons:
Specular highlights (shiny spots) are often missed because interpolation blurs them.
Lighting accuracy depends on vertex density.
- Use Cases:
Used in early real-time 3D games like PlayStation 1 and N64-era titles.

3. Phong Shading (1975 - Bui Tuong Phong)
- Concept:
Interpolates normals across the surface instead of interpolating colors.
Lighting is calculated per pixel rather than per vertex.
- Pros:
Produces much smoother shading and more accurate specular highlights.
Less dependent on polygon density.
- Cons:
More computationally expensive than Gouraud shading.
Still uses a simple empirical reflectance model.
- Use Cases:
Many modern non-PBR real-time graphics, including OpenGL's built-in Phong lighting.
Used in early 3D rendering like DirectX 8 games.

4. Blinn-Phong Shading (1977 - Jim Blinn)
- Concept:
A modification of Phong shading to improve efficiency.
Replaces the Phong specular calculation with the halfway vector (H = (L + V) / 2) instead of reflecting the light direction (R) and comparing with the view vector (V).
- Pros:
Faster than Phong shading.
Produces similar specular highlights.
- Cons:
Still an empirical (non-physically based) model.
Highlights look slightly different from Phong’s method.
- Use Cases:
Became the standard for real-time graphics in the 2000s (DirectX 9, OpenGL fixed pipeline).
Used in many older game engines before PBR became popular.

5. Cook-Torrance PBR (1982 - Robert Cook & Kenneth Torrance)
- Concept:
Physically-Based Rendering (PBR): Simulates real-world light interactions.
Uses microfacet theory: Surfaces consist of tiny reflective facets that scatter light.
Key components:
Bidirectional Reflectance Distribution Function (BRDF)
Fresnel Effect (Schlick’s approximation)
Microfacet Normal Distribution Function (NDF)
Geometric Shadowing Function (G)
- Pros:
More realistic than Blinn-Phong.
Works well under different lighting conditions.
Supports metallic and dielectric surfaces.
- Cons:
More computationally expensive.
Requires good asset creation (textures like roughness, metallic, AO maps).
- Use Cases:
Became the standard for modern game engines (Unreal Engine, Unity HDRP).
Used in films, high-end rendering, and modern graphics APIs (Vulkan, DirectX 12).