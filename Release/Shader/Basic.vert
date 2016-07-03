#version 330 core

in vec3 position;
in vec3 normal;
in vec2 uv;

out vec3 worldNormal;
out vec3 worldPosition;
out vec2 texCoord;
out vec4 ShadowCoord;

uniform mat4 model;
uniform mat4 view_proj;
uniform mat4 meshTrans;
uniform mat4 updateTrans;

uniform mat4 DepthBiasMVP;

uniform vec3 light;
uniform vec3 cameraPosition;
uniform vec3 diffuseColor;
uniform vec3 specularColor;
uniform int hasTexture;

void main()
{
	gl_Position = view_proj * model * meshTrans * updateTrans * vec4(position, 1); // h will not be stored, because always 1
	worldPosition = (model * meshTrans * updateTrans * vec4(position, 1)).rgb;//gl_Position.rgb;
	texCoord = uv;
	worldNormal = (model * meshTrans * updateTrans * vec4(normal, 0)).xyz;

	// Same, but with the light's view matrix
	ShadowCoord = DepthBiasMVP * meshTrans * updateTrans * vec4(position,1);
}