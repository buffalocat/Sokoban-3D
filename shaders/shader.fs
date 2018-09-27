#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D atlas;
uniform vec2 TexOffset;
uniform vec4 color;

void main()
{
    FragColor = texture(atlas, 0.25 * (TexCoord + TexOffset)) * color;
}
