# Image-Based Post-Processing Effects

![All Effects](featured.png?raw=true "Motion Blur, GTAO and Depth of Field")

This is the source code of my bachelors thesis "State of the Art in Image-Based Post-Processing Effects". It is written in C++, uses OpenGL 4.5 as graphics API and features:

- Physically based rendering (PBR)
- Baked probe-based global illumination
- Parallax corrected cubemaps for specular reflections (image based lighting)
- Shadowed point-, spot- and directional lights
- Bloom
- Some not so realistic lens flare and camera dirt effects

For my thesis, I implemented different techniques for image-based post-processing effects (depth of field, motion blur and SSAO), evaluated and compared them against one another to give an overview of the field and guidance for choosing a suitable technique for a given effect.

See https://doerriest.github.io/publication/bachelor/ for more details.