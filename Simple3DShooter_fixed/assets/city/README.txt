CITY BIOME
==========

A separate urban biome is procedurally generated in main.cpp.

- 26 skyscrapers with varied heights and facade styles
- Flat urban terrain for stable foundations
- Roads, sidewalks and a central civic plaza
- Each skyscraper uses one conservative axis-aligned AABB hitbox matching its footprint
- Decorative windows/crowns do not create extra collision geometry, preventing snagging
- City biome center: (260, -165), radius: 145 world units
- Concept art: city_biome_concept.png

The buildings are rendered procedurally from OpenGL primitives, so no external model loader is required.
