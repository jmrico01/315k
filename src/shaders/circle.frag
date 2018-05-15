#version 330 core

in vec2 vertPos;

out vec4 outColor;

uniform vec4 color;

void main()
{
    // NOTE vertex positions are [-0.5, 0.5]
    float distSq = vertPos.x * vertPos.x + vertPos.y * vertPos.y;
    if (distSq < 0.5 * 0.5) {
        outColor = color;
    }
    else {
    	outColor = vec4(0.0, 0.0, 0.0, 0.0);
    }
}