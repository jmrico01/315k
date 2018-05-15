#version 330 core

in vec2 vertPos;

out vec4 outColor;

uniform vec4 color;

void main()
{
    // NOTE vertex positions are [-0.5, 0.5]
    outColor = color * (1.0 - abs(vertPos.y) * 2.0);
}