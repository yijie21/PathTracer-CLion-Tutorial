#version 330

out vec4 color;
in vec2 TexCoords;

uniform sampler2D imgTex;

void main()
{
    color = texture(imgTex, TexCoords);
}