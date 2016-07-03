#version 330 core

in vec2 UV;
in vec4 colorParticle;

out vec4 color;

void main(){
	color = colorParticle;
}