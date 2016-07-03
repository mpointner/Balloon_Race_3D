#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D image;
uniform bool horizontal;

uniform float gaussianWeight[5] = float[] (0.2270270270, 0.1945945946, 0.1216216216, 0.0540540541, 0.0162162162);

void main()
{             
     vec2 sizeOfOnePixel = 1.0 / textureSize(image, 0); // Size of single texel
     vec3 result = texture(image, TexCoords).rgb * gaussianWeight[0];
     if(horizontal)
     {
         for(int i = 1; i < 5; ++i)
         {
            result += texture(image, TexCoords + vec2(sizeOfOnePixel.x * i, 0.0)).rgb * gaussianWeight[i];
            result += texture(image, TexCoords - vec2(sizeOfOnePixel.x * i, 0.0)).rgb * gaussianWeight[i];
         }
     }
     else
     {
         for(int i = 1; i < 5; ++i)
         {
             result += texture(image, TexCoords + vec2(0.0, sizeOfOnePixel.y * i)).rgb * gaussianWeight[i];
             result += texture(image, TexCoords - vec2(0.0, sizeOfOnePixel.y * i)).rgb * gaussianWeight[i];
         }
     }
     FragColor = vec4(result, 1.0);
}