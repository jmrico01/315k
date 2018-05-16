#version 330 core

in vec2 fragUV;

out vec4 outColor;

uniform sampler2D framebufferTexture;

#define THRESHOLD 0.5

float ColorToLuminance(vec3 color) {
    // Luminance color weights source:
    // https://stackoverflow.com/questions/596216/formula-to-determine-brightness-of-rgb-color
    return dot(color, vec3(0.2126, 0.7152, 0.0722));
}

void main()
{
    vec3 color = texture(framebufferTexture, fragUV).rgb;
    float lum = ColorToLuminance(color);
    if (lum >= THRESHOLD) {
        outColor = vec4(color, 1.0);
    }
    else {
        outColor = vec4(0.0, 0.0, 0.0, 0.0);
    }
}