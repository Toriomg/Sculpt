#shader vertex
#version 330 core
layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texCoord;
layout(location = 2) in vec3 normal;

uniform mat4 u_MVP;
uniform mat4 u_Model;
uniform sampler2D u_Textures[2];

out vec4 v_Color;
out vec2 v_TexCoord;
out vec3 v_Normal;
out vec3 v_WorldPos;


void main()
{
	gl_Position = u_MVP * vec4(position, 1.0);
	v_TexCoord = texCoord;
	v_Normal = normal;

	v_WorldPos = vec3(u_Model * vec4(position, 1.0));
}

#shader fragment
#version 330 core

in vec4 v_Color;
in vec2 v_TexCoord;
in vec3 v_Normal;
in vec3 v_WorldPos;

out vec4 FragColor;

uniform sampler2D u_Textures[2];
uniform vec3 u_cameraPos;

void main()
{
	vec3 objectColor = texture(u_Textures[0], v_TexCoord).rgb;

	vec3 faceNormal = normalize(cross(dFdx(v_WorldPos), dFdy(v_WorldPos)));

	// 3. Define light properties
	vec3 lightColor = vec3(1.0, 1.0, 1.0); // A simple white light
	vec3 lightPos = u_cameraPos;

	// 4. Calculate diffuse lighting (Lambertian reflection)
	vec3 lightDir = normalize(lightPos - v_WorldPos);
	float diff = max(dot(faceNormal, lightDir), 0.0);
	vec3 diffuse = diff * lightColor;

	// 5. Add a little bit of ambient light so things in shadow aren't pure black
	float ambientStrength = 0.1;
	vec3 ambient = ambientStrength * lightColor;

	// 6. Combine lighting and the object's color
	vec3 result = (ambient + diffuse) * objectColor;

	// Final color output
	FragColor = vec4(result, 1.0);
}