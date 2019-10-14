#version 430

layout (location = 0) in vec2 texCoord;
layout (location = 0) out vec4 fragColor;
layout (binding = 0) uniform sampler2D objectTexture;

uniform vec4 color;
uniform bool useTexture;

void main(void)
{
	fragColor = color;
	
	if (useTexture) {
		int mipMapLevel = int(textureQueryLod(objectTexture, texCoord).x);
		fragColor = textureLod(objectTexture, texCoord, mipMapLevel);
		//fragColor = texture(objectTexture, texCoord);
	}
	
	if (fragColor.a == 0) {
		discard;
	}
}
