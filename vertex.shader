#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in vec3 aNormal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;
uniform vec3 texture_scale;
uniform vec2 texture_pixel_scale;

out vec2 tex_coords;

void main()
{
	gl_Position = proj * view * model * vec4(aPos, 1.0);

	vec3 Normals = vec3(aNormal.x, aNormal.y, aNormal.z);
	if (Normals.x != 0.0)
	{
		tex_coords = vec2(aTexCoord.x * texture_scale.z * texture_pixel_scale.x, aTexCoord.y * texture_scale.y * texture_pixel_scale.y);
	}
	if (Normals.y != 0.0)
	{
		tex_coords = vec2(aTexCoord.x * texture_scale.x * texture_pixel_scale.x, aTexCoord.y * texture_scale.z * texture_pixel_scale.y);
	}
	if (Normals.z != 0.0)
	{
		tex_coords = vec2(aTexCoord.x * texture_scale.x * texture_pixel_scale.x, aTexCoord.y * texture_scale.y * texture_pixel_scale.y);
	}
}