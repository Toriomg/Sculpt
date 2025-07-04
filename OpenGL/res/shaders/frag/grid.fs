#version 330 core

in vec3 v_worldPos;
uniform vec3 u_cameraPos;
out vec4 FragColor;

float grid(vec2 coord, float width) {
     vec2 fw = fwidth(coord);
    // We clamp the width to avoid lines becoming too thick up close.
    vec2 clamped_fw = clamp(fw, 0.0, 0.01 * width);
    vec2 grid_uv = fract(coord);
    vec2 line = smoothstep(clamped_fw * width, vec2(0.0), grid_uv) - smoothstep(vec2(1.0) - clamped_fw * width, vec2(1.0), grid_uv);
    return 1.0 - max(line.x, line.y);
}

void main()
{
    vec3 direction = normalize(v_worldPos - u_cameraPos);
    float t = -u_cameraPos.y / direction.y;

    if (t < 0.0) {
        discard;
    }

    vec3 groundPos = u_cameraPos + direction * t;
    float dist = length(groundPos - u_cameraPos);
    
    float tinyLine = grid(groundPos.xz / 1.0, 1.0);
    float midLine = grid(groundPos.xz / 10.0, 2.0); 
    float majorLine = grid(groundPos.xz / 100.0, 5.5);
    
    vec2 tiny_fw = fwidth(groundPos.xz);
    vec2 mid_fw = fwidth(groundPos.xz / 10.0);

    float tiny_vis = smoothstep(1.5, 0.7, max(tiny_fw.x, tiny_fw.y));
    float mid_vis = smoothstep(1.5, 0.7, max(mid_fw.x, mid_fw.y));

    float gridLines = 1.0; // Start with a blank slate (1.0 = no line)
    gridLines = min(gridLines, mix(1.0, tinyLine, tiny_vis));
    gridLines = min(gridLines, mix(1.0, midLine, mid_vis));
    gridLines = min(gridLines, majorLine); // Major grid is always on

    vec3 lineColor = vec3(0.0);
    float lineAlpha = 1.0 - gridLines;
    vec4 gridSourceColor = vec4(lineColor, lineAlpha);

    float fogFactor = 1.0 - exp(-dist * 0.005);
    vec4 fogColor = vec4(0.2f, 0.3f, 0.3f, 1.0f); // Background color

    gridSourceColor.a *= 1.0 - fogFactor;
    FragColor = gridSourceColor;
}