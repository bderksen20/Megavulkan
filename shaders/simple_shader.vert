#version 450

// Hardcoded setup triangle vertices
//vec2 positions[3] = vec2[] (
//	vec2(0.0, -0.5),
//	vec2(0.5, 0.5),
//	vec2(-0.5, 0.5)
//);

// Specified first vertex attribute
// in = takes value from a vertex buffer
layout(location = 0) in vec4 position;
layout(location = 1) in vec3 color;

// Removed for push constants tutorial
layout(location = 0) out vec3 fragColor;

// Order here must match struct order
layout(push_constant) uniform Push {
	mat4 world_transform;
	//vec2 offset;
	//vec3 color;
} push;

// Uniform buffer 
layout(binding = 0) uniform UniformBuffo{
	mat4 model;
	mat4 view;
	mat4 proj;
} ubo;

void main() {

	//gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);	// gl_VertexIndex appears as error in VS19, but compiles fine
	//gl_Position = vec4(position, 0.0, 1.0);
	//gl_Position = vec4(push.transform * position + push.offset, 0.0, 1.0);
	//vec4 pos = push.world_transform * position;
	//gl_Position = ubo.proj * ubo.view * ubo.model * vec4(push.world_transform * position);
	gl_Position = ubo.proj * ubo.view * ubo.model * vec4(position);

	// Removed for push constants
	fragColor = color;
}