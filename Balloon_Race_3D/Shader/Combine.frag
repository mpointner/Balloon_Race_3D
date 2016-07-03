#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D scene;
uniform sampler2D bloomBlur;
uniform bool bloom;

void main()
{           
    const float gamma = 1.5;
    vec3 hdrColor = texture(scene, TexCoords).rgb;
    vec3 bloomColor = texture(bloomBlur, TexCoords).rgb;
	vec3 result;
    if(bloom) 
	{
        hdrColor += bloomColor; // additive blending
	}
	FragColor = vec4(hdrColor,1);
}