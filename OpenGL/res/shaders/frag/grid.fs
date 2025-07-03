#version 330 core

in vec3 v_worldPos;
uniform vec3 u_cameraPos; // You MUST send this from C++

layout(location = 0) out vec4 FragColor;

// ... (your helper functions: satf, maxVec2, log10) ...
vec2 satv(vec2 x) { return clamp(x, vec2(0.0), vec2(1.0)); }
float satf(float x) { return clamp(x, 0.0, 1.0); }
float maxVec2(vec2 v) { return max(v.x, v.y); }
float log10(float x) { return log(x) / log(10.0); }

// These can be uniforms too
const float gGridCellSize = 1.0;
const float gGridMinPixelsBetweenCells = 2.0;
const vec4 gGridColorThin = vec4(0.3, 0.3, 0.3, 1.0);
const vec4 gGridColorThick = vec4(0.5, 0.5, 0.5, 1.0);
const vec4 u_fog_color = vec4(0.2, 0.3, 0.3, 1.0); // Match your glClearColor

// Re-usable LOD function
float Lod(vec3 position, float GridCellSizeLod){
    vec2 grid_uv = position.xz / GridCellSizeLod;
    vec2 mod_div_dudv = fract(grid_uv);
    vec2 line = abs(mod_div_dudv * 2.0 - 1.0);
    return max(1.0 - line.x, 1.0 - line.y);
}

void main()
{
    // --- FIX #2: PROJECT TO FLOOR PLANE (y=0) ---
    vec3 direction = normalize(v_worldPos - u_cameraPos);
    if (direction.y >= -1e-6 || u_cameraPos.y <= 0.0) {
        discard; // Discard "sky" pixels
    }
    float t = -u_cameraPos.y / direction.y;
    vec3 groundPos = u_cameraPos + direction * t; // Use this for all calcs!

    // --- FIX #1: CORRECT DERIVATIVE CALCULATION ---
    float l = length(fwidth(groundPos.xz)); // Use fwidth on the final coordinate space

	float LOD = max(0.0 , log10(l * gGridMinPixelsBetweenCells / gGridCellSize) + 1.0);

	float GridCellSizeLod0 = gGridCellSize * pow(10.0, floor(LOD));
	float GridCellSizeLod1 = GridCellSizeLod0 * 10.0;
	float GridCellSizeLod2 = GridCellSizeLod1 * 10.0;

	float Lod0a = Lod(groundPos, GridCellSizeLod0);
	float Lod1a = Lod(groundPos, GridCellSizeLod1);
	float Lod2a = Lod(groundPos, GridCellSizeLod2);

	float LOD_fade = fract(LOD);

	vec4 Color;
    float finalAlpha;

    // This logic correctly blends between LODs
	if (Lod1a > 0.0) { // If the thickest lines are visible
        Color = mix(gGridColorThin, gGridColorThick, LOD_fade);
        finalAlpha = Lod1a;
    } else { // Otherwise, show the thinnest lines and fade them out
        Color = gGridColorThin;
        finalAlpha = Lod0a * (1.0 - LOD_fade);
    }

    // Apply distance falloff
	float OpacityFalloff = 1.0 - satf(length(groundPos - u_cameraPos) / 500.0);
	finalAlpha *= OpacityFalloff;

    // Blend final grid color with the fog
	FragColor = mix(u_fog_color, Color, finalAlpha);
}