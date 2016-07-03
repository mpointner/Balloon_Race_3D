#version 330 core

layout(location=0) out vec4 fragColor;

in vec3 worldNormal;
in vec2 texCoord;
in vec3 worldPosition;
in vec4 ShadowCoord;

uniform vec3 lightDirection;
uniform vec3 cameraPosition;
uniform vec3 diffuseColor;
uniform vec3 specularColor;
uniform int hasTexture;

uniform sampler2D color_texture;

uniform sampler2DShadow shadowMap;

vec3 sub(vec3 v1, vec3 v2);
float dot(vec3 v1, vec3 v2);
vec3 mul(vec3 v, float f);
float max(float x, float y);
vec3 norm(vec3 v);
float length(vec3 v);

void main()
{
	vec3 v = norm(sub(cameraPosition, worldPosition));
	vec3 l = norm(lightDirection);
	vec3 h = norm(v + l);
	vec3 n = norm(worldNormal);
	float cos = max(dot(n, l), 0) * 0.8f + 0.2f;


	float visibility = texture( shadowMap, vec3(ShadowCoord.xy, (ShadowCoord.z)/ShadowCoord.w) );

	//*
	if(hasTexture == 1) {
		fragColor = vec4(texture(color_texture, texCoord).rgb, 1.0) * visibility;
	} else {
		fragColor = (vec4((0.8 * diffuseColor * cos) + (1 * specularColor * pow(max(dot(n, h), 0), 50)), 1.0)) * visibility;
	}
	//*/
	//fragColor = vec4(vec3(texture(shadowMap, worldPosition)), 1.0);
	//fragColor = ShadowCoord.xyzw;
	//visibility = texture( shadowMap, vec3(ShadowCoord.xy, (ShadowCoord.z)) );
	//fragColor = vec4(visibility, visibility, visibility, 1);
	//fragColor = vec4(ShadowCoord.x, ShadowCoord.y, ShadowCoord.z, 1);
}

float max(float x, float y)
{
	return x > y ? x : y;
}

float dot(vec3 v1, vec3 v2)
{
	return v1.r * v2.r + v1.g * v2.g + v1.b * v2.b;
}

vec3 mul(vec3 v, float f)
{
	return vec3(v.r * f, v.g * f, v.b * f);
}

vec3 sub(vec3 v1, vec3 v2)
{
	return vec3(v1.r - v2.r, v1.g - v2.g, v1.b - v2.b); 
}

vec3 norm(vec3 v)
{
	float l = length(v);
	return vec3(v.r / l, v.g / l, v.b / l);
}

float length(vec3 v)
{
	return sqrt(v.r * v.r + v.g * v.g + v.b * v.b);
}