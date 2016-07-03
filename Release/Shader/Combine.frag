#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D scene;
uniform sampler2D bloomBlur;
uniform bool bloom;
uniform float exposure;
uniform bool hdrEnabled;

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
	if(hdrEnabled)
	{
		// tone mapping
		result = vec3(1.0) - exp(-hdrColor * exposure);
		// Gamma correction 
		result = pow(result, vec3(1.0 / gamma));
	}
	else
	{
		result =  hdrColor;
	}
	FragColor = vec4(result,1);
}