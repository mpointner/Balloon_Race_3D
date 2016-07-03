#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texCoords;

out vec2 TexCoords;
out vec4 avgLuminance;

uniform sampler2D scene;

void main()
{
    gl_Position = vec4(position, 1.0f);
    TexCoords = texCoords;

	vec2 zeroVec = vec2(0, 0);
	vec2 sizeVec = textureSize(scene, 0);
	float mipmapLevel = 1 + floor(log2(max(sizeVec.x, sizeVec.y)));
	float resize = mipmapLevel/10.0f;
	avgLuminance = textureLod(scene, zeroVec, mipmapLevel); 

}