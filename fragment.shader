#version 460 core
out vec4 fragColor;

in vec2 tex_coords;

uniform sampler2D tex;
uniform bool highlighted = false;

void main()
{
    vec4 color = texture(tex, tex_coords);
    if (highlighted)
    {
        color *= 2.0;
    }
    fragColor = color;
}