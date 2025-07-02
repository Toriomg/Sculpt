#version 330 core

in vec3 v_worldPos;
layout(location = 0) out vec4 FragColor;

uniform float gGtidCellSize = 5.025f; // Size of each grid cell in world units
uniform vec4 gGridColorThin = vec4(0.0, 0.0, 0.0, 1.0); // Color for thin grid lines
uniform vec4 gGridColorThick = vec4(0.5, 0.5, 0.5, 1.0); // Color for thick grid lines

void main()
{
	float Lod0a = mod(v_worldPos.z, gGtidCellSize);

	vec4 Color;
	
	Color = gGridColorThick;
	Color.a *= Lod0a;

	FragColor = Color;
}