#version 450

// Hardcoded setup triangle vertices
//vec2 positions[3] = vec2[] (
//	vec2(0.0, -0.5),
//	vec2(0.5, 0.5),
//	vec2(-0.5, 0.5)
//);

// Hardcoded setup vertex colors!!!
//vec3 colors[3] = vec3[](
//	vec3(1.0, 0.0, 0.0),
//	vec3(0.0, 1.0, 0.0),
//	vec3(0.0, 0.0, 1.0)
//);

// Specified first vertex attribute
// in = takes value from a vertex buffer
layout(location = 0) in vec2 position;
layout(location = 1) in vec3 color;

// Removed for push constants tutorial
//layout(location = 0) out vec3 fragColor;

// Order here must match struct order
layout(push_constant) uniform Push {
	mat2 transform;
	vec2 offset;
	vec3 color;
} push;

void main() {

	//gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);	// gl_VertexIndex appears as error in VS19, but compiles fine
	//gl_Position = vec4(position, 0.0, 1.0);
	gl_Position = vec4(push.transform * position + push.offset, 0.0, 1.0);

	// Removed for push constants
	//fragColor = color;
}