#version 330 core

in vec3 v_worldPos;
uniform vec3 u_cameraPos;
out vec4 FragColor;

float grid(vec2 coord, float thickness) {
    vec2 grid_uv = fract(coord);
    vec2 line = step(vec2(thickness), grid_uv) - step(vec2(1.0 - thickness), grid_uv);
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
    
    float lineThickness = 0.01;
    float minorLine = grid(groundPos.xz, lineThickness);
    float majorLine = grid(groundPos.xz / 10.0, lineThickness * 2.0);
    
    vec3 gridColor = vec3(0.5);
    float gridLines = max(minorLine, majorLine);
    vec3 finalColor = gridColor * gridLines;

    float fogFactor = 1.0 - exp(-dist * 0.005);
    vec4 fogColor = vec4(0.1, 0.1, 0.1, 1.0); // Background color

    FragColor = mix(vec4(finalColor, gridLines), fogColor, fogFactor);
}