#shader vertex
#version 330 core
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoord;

uniform mat4 u_ViewProjection;
uniform mat4 u_Model;
uniform sampler2D u_Texture;
uniform vec4 u_Color;

out vec4 v_Color;
out vec2 v_TexCoord;
out vec3 v_Normal;
out vec3 v_WorldPos;

void main()
{
	gl_Position = u_ViewProjection * u_Model * vec4(position, 1.0);
	v_TexCoord = texCoord;

	v_WorldPos = vec3(u_Model * vec4(position, 1.0));
	v_Normal = mat3(transpose(inverse(u_Model))) * normal;
	v_Color = u_Color;
}

#shader fragment
#version 330 core

in vec4 v_Color;
in vec2 v_TexCoord;
in vec3 v_Normal;
in vec3 v_WorldPos;

out vec4 FragColor;

uniform sampler2D u_Texture;
uniform vec3 u_cameraPos;
uniform vec3 u_objectColor;


// Model selection uniforms
uniform bool u_IsSelected;
uniform vec4 u_HighlightColor;
// Triangle selection uniforms
uniform bool u_IsTriangleSelected;
uniform int u_SelectedTriangleID;
uniform vec4 u_TriangleHighlightColor;
// Vertex selection uniforms
uniform bool u_IsVertexSelected;
uniform vec3 u_SelectedVertexWorldPos;
uniform vec4 u_VertexHighlightColor;
uniform float u_VertexHighlightRadius;


vec3 lighting(){
	vec3 norm = normalize(v_Normal);
	vec3 faceNormal = normalize(cross(dFdx(v_WorldPos), dFdy(v_WorldPos)));

	// Define light properties
	vec3 lightColor = vec3(0.6); // A simple white light
	vec3 lightPos = u_cameraPos;//vec3(0.0, 30.0, 0.0);

	// DIFFUSE LIGHT
	// Calculate diffuse lighting (Lambertian reflection)
	vec3 lightDir = normalize(lightPos - v_WorldPos);
	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = diff * lightColor;

	// AMBIENT LIGHT
	// Add a little bit of ambient light so things in shadow aren't pure black
	float ambientStrength = 0.30;
	vec3 ambient = ambientStrength * lightColor;

	// SPECULAR light
	float specularStrength = 0.7; // Controls the intensity of the highlight
	float shininess = 20.0;       // Controls the size of the highlight (higher is smaller/sharper)

	// Calculate the direction the viewer is looking from (fragment to camera)
	vec3 viewDir = normalize(u_cameraPos  - v_WorldPos);
	
	// Calculate the reflection direction of the light off the surface
	// reflect() expects the direction *from* the light source, so we use -lightDir
	//vec3 reflectDir = reflect(-lightDir, norm);

	// Calculate the specular component
	vec3 halfwayDir = normalize(lightDir + viewDir);
	float spec = pow(max(dot(norm, halfwayDir), 0.0), shininess);
	vec3 specular = specularStrength * spec * vec3(1.0);
	//specular = specular * u_objectColor;

	return ambient * v_Color.rgb + diffuse * v_Color.rgb + specular;
}

void main()
{
	/*
	vec4 texColor = texture(u_Texture, v_TexCoord);
	FragColor = vec4(v_TexCoord.x, v_TexCoord.y, 0.0, 1.0);
	*/
	vec3 objectColor = texture(u_Texture, v_TexCoord).rgb;
	
	vec3 baseColor;

	if (u_IsSelected) {
        // mix() interpola entre dos colores. Un factor de 0.5 mezcla ambos a partes iguales.
        baseColor = mix(objectColor, u_HighlightColor.rgb, 0.5); 
    }
    else {
        baseColor = objectColor;
    }

	if (u_IsTriangleSelected && gl_PrimitiveID  == u_SelectedTriangleID) {
        // Use a stronger mix for the triangle highlight to make it stand out
        baseColor = mix(baseColor, u_TriangleHighlightColor.rgb, 0.85);
    }

	// Combine lighting and the object's color
	vec3 result = baseColor *lighting();

	if (u_IsVertexSelected) {
        // Calculate the distance from the current pixel's world position to the center of the selected vertex.
        float dist = distance(v_WorldPos, u_SelectedVertexWorldPos);

        // Smoothstep to create a soft-edged circle.
        // It will be fully opaque inside (radius - 0.5) and fade to transparent at the radius edge.
        float falloff = 0.5;
        float highlightFactor = 1.0 - smoothstep(u_VertexHighlightRadius - falloff, u_VertexHighlightRadius, dist);
        
        // If the pixel is inside the circle's influence...
        if (highlightFactor > 0.0) {
            // Mix the final lit color with the vertex highlight color.
            result = mix(result, u_VertexHighlightColor.rgb, highlightFactor);
        }
    }

	// Final color output
	FragColor = vec4(objectColor, 1.0);
}