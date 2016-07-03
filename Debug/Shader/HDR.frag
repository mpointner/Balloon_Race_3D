#version 330 core
layout (location = 0) out vec4 FragColor;
in vec2 TexCoords;
in vec4 avgLuminance;

uniform sampler2D scene;
uniform int hdrEnabled;

void main()
{           
    const float gamma = 1.5;
    vec3 hdrColor = texture(scene, TexCoords).rgb;
	vec3 result = hdrColor;

	if(hdrEnabled == 1)
	{
		float luminance = 0.28 * avgLuminance.x + 0.65 * avgLuminance.y + 0.07 * avgLuminance.z;
		float a = 0.2;
		vec3 L_xy = (a * hdrColor) / luminance;
		result = L_xy / (L_xy + vec3(1.0));
		// gamma correction
		result = pow(result, vec3(1.0 / gamma));
	}

	//FragColor = vec4(hdrColor,1);
	FragColor = vec4(result, 1.0f);
}