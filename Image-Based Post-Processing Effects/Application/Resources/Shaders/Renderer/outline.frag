#version 450 core

layout(location = 0) out vec4 oAlbedo;
layout(location = 1) out vec4 oNormal;
layout(location = 2) out vec4 oMetallicRoughnessAo;

uniform vec4 uOutlineColor;

void main()
{
	oAlbedo = uOutlineColor;
	oNormal = vec4(0.0);
	oMetallicRoughnessAo = vec4(0.0);
}

