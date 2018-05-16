#version 330 core

in vec2 fragUV;

out vec4 outColor;

uniform sampler2D framebufferTexture;
uniform vec2 screenSize;
uniform int isHorizontal;

#define KERNEL_HALFSIZE 3
#define KERNEL_SIZE (KERNEL_HALFSIZE * 2 + 1)

const float GAUSSIAN_KERNEL[KERNEL_SIZE] = float[KERNEL_SIZE](
    0.071303, 0.131514, 0.189879, 0.214607, 0.189879, 0.131514, 0.071303
);

void main()
{
    ivec2 step = ivec2(1, 0);
    if (isHorizontal == 0) {
        step = ivec2(0, 1);
    }

    vec3 color = vec3(0.0, 0.0, 0.0);
    for (int i = -KERNEL_HALFSIZE; i <= KERNEL_HALFSIZE; i++) {
        vec3 c = textureOffset(framebufferTexture, fragUV,
            step * i).rgb;
        color += c * GAUSSIAN_KERNEL[i + KERNEL_HALFSIZE];
    }

    outColor = vec4(color, 1.0);
}