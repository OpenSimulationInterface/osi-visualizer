#version 430

layout (location = 0) in vec3 vertex;
layout (location = 1) in vec2 textureCoordinateIn;
layout (location = 0) out vec2 textureCoordinateOut;

uniform mat4 mvpMatrix;

void main(void)
{
	textureCoordinateOut = textureCoordinateIn;
	gl_Position = mvpMatrix * vec4(vertex, 1.0);
}
