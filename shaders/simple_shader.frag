#version 450

layout (location = 0) out vec4 outColor;

// Removed for push constants tutorial
layout (location = 0) in vec3 fragColor;

// Order here must match struct order

layout(push_constant) uniform Push {
	mat4 world_transform;
	//vec2 offset;
	//vec3 color;
} push;

void main() {

	// Red, green, blue, and alpha channels (0-1 range)
	// outColor = vec4(0.0, 1.0, 0.0, 1.0);
	outColor = vec4(fragColor, 1.0);

}