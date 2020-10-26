#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(constant_id = 0) const float SCREEN_WIDTH = 0.0;
layout(constant_id = 1) const float SCREEN_HEIGHT = 0.0;

layout(push_constant) uniform ArrayInfo
{
	uint size;
} array_info;

layout(location = 0) in uint value;
layout(location = 1) in uint initial_position;
layout(location = 0) out vec4 frag_color;

const uint indices[6] = uint[]
(
	0, 1, 2,
	2, 3, 1
);

const vec2 vertices[4] = vec2[]
(
	vec2(0, 0), vec2(1, 0),
	vec2(0, 1), vec2(1, 1)
);

void main()
{
	const float bar_width = SCREEN_WIDTH / float(array_info.size);
	const float base_position = float(gl_VertexIndex) / float(array_info.size);

	vec2 bar_position = base_position + vertices[indices[gl_InstanceIndex]];
	bar_position.y *= SCREEN_HEIGHT;
	bar_position.x *= bar_width;

	gl_Position = vec4(bar_position, 0.0, 1.0);

	frag_color = vec4(1.0);
}