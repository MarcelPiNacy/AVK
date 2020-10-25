#version 450
#extension GL_ARB_separate_shader_objects : enable

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
	gl_Position = vec4(vertices[indices[gl_InstanceIndex]], 0.0, 1.0);
	frag_color = vec4(1.0);
}