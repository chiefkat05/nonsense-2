#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

uniform vec2 texture_size;

out vec2 tex_coords;

void main()
{
	gl_Position = proj * view * model * vec4(aPos, 1.0);

	tex_coords = aTexPos / texture_size;
}