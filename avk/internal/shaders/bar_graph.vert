#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(push_constant) uniform ShaderArgs
{
	size_t array_size;
} args;

layout(location = 0) in size_t	item_value;
layout(location = 1) in size_t	item_original_position;
layout(location = 2) in vec3	item_color;
layout(location = 3) in size_t	item_flags;
layout(location = 0) out vec4	out_frag_color;

const vec2 local_vertices[4] = vec2[]
(
	vec2(1, 1), vec2(0, 1),
	vec2(0, 0), vec2(1, 0)
);

const size_t local_indices[6] = size_t[](0, 1, 2, 2, 3, 0);

void main()
{
	const vec2 base_vertex = local_vertices[local_indices[gl_VertexIndex]];
	vec2 v = base_vertex;
	size_t item_index = gl_InstanceIndex;

	float relative_original_position = float(item_original_position) / float(args.array_size);
	float relative_item_value = float(item_value) / float(args.array_size);

	float bar_width = 2.0 / float(args.array_size);
	float bar_height = 2.0 * relative_item_value;

	v.x *= bar_width;
	v.y *= bar_height;
	v.x += item_index * bar_width;
	--v.x;
	--v.y;
	v.y += 2.0 * (1.0 - relative_item_value);

	gl_Position = vec4(v, 0.0, 1.0);

	if ((item_flags & 1) == 0)
	{
		relative_original_position *= relative_original_position;
		if (base_vertex.y == 1)
			out_frag_color = vec4(vec3(relative_original_position), 1.0);
		else
			out_frag_color = vec4(item_color, 1.0);
	}
	else
	{
		out_frag_color = vec4(1, 0, 0, 1);
	}
}