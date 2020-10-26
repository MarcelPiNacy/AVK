#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(constant_id = 0) const float INV_SCREEN_WIDTH = 0.0;
layout(constant_id = 1) const float INV_SCREEN_HEIGHT = 0.0;

layout(location = 0) in vec4 frag_color;
layout(location = 0) out vec4 out_color;

void main()
{
	vec4 tmp = frag_color;
	tmp = gl_FragCoord;
	out_color = tmp;
}