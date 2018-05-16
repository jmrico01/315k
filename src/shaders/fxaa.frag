#version 330 core

in vec2 fragUV;

out vec4 outColor;

uniform sampler2D framebufferTexture;
uniform vec2 invScreenSize;

//#define EDGE_THRESHOLD_MIN 0.0312
//#define EDGE_THRESHOLD_MAX 0.125
#define EDGE_THRESHOLD_MIN 0.0
#define EDGE_THRESHOLD_MAX 0.0

#define ITERATIONS 12
#define QUALITY(i) (quality_[i])
#define SUBPIXEL_QUALITY 0.75

const float quality_[ITERATIONS] = float[ITERATIONS](
    1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
    1.5, 2.0, 2.0, 2.0, 4.0, 8.0
);

float ColorToLuminance(vec3 color) {
    return sqrt(dot(color, vec3(0.299, 0.587, 0.114)));
}

void main()
{
    // Wonderful, very complete FXAA algorithm. Source:
    // http://blog.simonrodriguez.fr/articles/30-07-2016_implementing_fxaa.html
    vec3 color = texture(framebufferTexture, fragUV).rgb;
    float lumMid = ColorToLuminance(color);
    float lumLeft = ColorToLuminance(textureOffset(framebufferTexture,
        fragUV, ivec2(-1, 0)).rgb);
    float lumRight = ColorToLuminance(textureOffset(framebufferTexture,
        fragUV, ivec2(1, 0)).rgb);
    float lumBot = ColorToLuminance(textureOffset(framebufferTexture,
        fragUV, ivec2(0, -1)).rgb);
    float lumTop = ColorToLuminance(textureOffset(framebufferTexture,
        fragUV, ivec2(0, 1)).rgb);

    float lumMin = min(lumMid,
        min(min(lumLeft, lumRight), min(lumBot, lumTop)));
    float lumMax = max(lumMid,
        max(max(lumLeft, lumRight), max(lumBot, lumTop)));
    float lumDelta = lumMax - lumMin;

    if (lumDelta < max(EDGE_THRESHOLD_MIN, lumMax * EDGE_THRESHOLD_MAX)) {
        // No edge, don't anti-alias
        outColor = vec4(color, 1.0);
        return;
    }

    float lumBotLeft = ColorToLuminance(textureOffset(framebufferTexture,
        fragUV, ivec2(-1, -1)).rgb);
    float lumBotRight = ColorToLuminance(textureOffset(framebufferTexture,
        fragUV, ivec2(1, -1)).rgb);
    float lumTopLeft = ColorToLuminance(textureOffset(framebufferTexture,
        fragUV, ivec2(-1, 1)).rgb);
    float lumTopRight = ColorToLuminance(textureOffset(framebufferTexture,
        fragUV, ivec2(1, 1)).rgb);

    float lumBotTop = lumBot + lumTop;
    float lumLeftRight = lumLeft + lumRight;

    float lumBotCorners = lumBotLeft + lumBotRight;
    float lumTopCorners = lumTopLeft + lumTopRight;
    float lumLeftCorners = lumBotLeft + lumTopLeft;
    float lumRightCorners = lumBotRight + lumTopRight;

    float edgeHorizontal = abs(-2.0 * lumLeft + lumLeftCorners)
        + abs(-2.0 * lumMid + lumBotTop) * 2.0
        + abs(-2.0 * lumRight + lumRightCorners);
    float edgeVertical = abs(-2.0 * lumTop + lumTopCorners)
        + abs(-2.0 * lumMid + lumLeftRight) * 2.0
        + abs(-2.0 * lumBot + lumBotCorners);

    // Is the local edge horizontal or vertical?
    bool isHorizontal = (edgeHorizontal >= edgeVertical);

    // Select the two neighboring texels lums
    // in the opposite direction to the local edge.
    float lum1 = isHorizontal ? lumBot : lumLeft;
    float lum2 = isHorizontal ? lumTop : lumRight;
    // Compute gradients in this direction.
    float gradient1 = lum1 - lumMid;
    float gradient2 = lum2 - lumMid;

    // Which direction is the steepest?
    bool is1Steepest = abs(gradient1) >= abs(gradient2);

    // Gradient in the corresponding direction, normalized.
    float gradientScaled = 0.25 * max(abs(gradient1), abs(gradient2));

    // Choose the step size (one pixel) according to the edge direction.
    float stepLength = isHorizontal ?
        invScreenSize.y : invScreenSize.x;

    // Average lum in the correct direction.
    float lumLocalAverage = 0.0;

    if (is1Steepest) {
        // Switch the direction
        stepLength = - stepLength;
        lumLocalAverage = 0.5 * (lum1 + lumMid);
    } else {
        lumLocalAverage = 0.5 * (lum2 + lumMid);
    }

    // Shift UV in the correct direction by half a pixel.
    vec2 currentUV = fragUV;
    if (isHorizontal) {
        currentUV.y += stepLength * 0.5;
    } else {
        currentUV.x += stepLength * 0.5;
    }

    // Compute offset (for each iteration step) in the right direction.
    vec2 offset = isHorizontal ?
        vec2(invScreenSize.x, 0.0) : vec2(0.0, invScreenSize.y);
    // Compute UVs to explore on each side of the edge, orthogonally.
    // The QUALITY allows us to step faster.
    vec2 uv1 = currentUV - offset;
    vec2 uv2 = currentUV + offset;

    // Read the lums at both current extremities of the exploration segment,
    // and compute the delta wrt to the local average lum.
    float lumEnd1 = ColorToLuminance(texture(framebufferTexture, uv1).rgb);
    float lumEnd2 = ColorToLuminance(texture(framebufferTexture, uv2).rgb);
    lumEnd1 -= lumLocalAverage;
    lumEnd2 -= lumLocalAverage;

    // If the lum deltas at the current extremities are larger than
    // the local gradient, we have reached the side of the edge.
    bool reached1 = abs(lumEnd1) >= gradientScaled;
    bool reached2 = abs(lumEnd2) >= gradientScaled;
    bool reachedBoth = reached1 && reached2;

    // If the side is not reached, we continue to explore in this direction.
    if (!reached1) {
        uv1 -= offset;
    }
    if (!reached2) {
        uv2 += offset;
    }
    
    // If both sides have not been reached, continue to explore.
    if (!reachedBoth) {
        for (int i = 2; i < ITERATIONS; i++) {
            // If needed, read lum in 1st direction, compute delta.
            if (!reached1) {
                lumEnd1 = ColorToLuminance(texture(
                    framebufferTexture, uv1).rgb);
                lumEnd1 = lumEnd1 - lumLocalAverage;
            }
            // If needed, read lum in opposite direction, compute delta.
            if (!reached2) {
                lumEnd2 = ColorToLuminance(texture(
                    framebufferTexture, uv2).rgb);
                lumEnd2 = lumEnd2 - lumLocalAverage;
            }
            // If the lum deltas at the current extremities is larger than
            // the local gradient, we have reached the side of the edge.
            reached1 = abs(lumEnd1) >= gradientScaled;
            reached2 = abs(lumEnd2) >= gradientScaled;
            reachedBoth = reached1 && reached2;

            // If the side is not reached, we continue to explore
            // in this direction, with a variable quality.
            if (!reached1) {
                uv1 -= offset * QUALITY(i);
            }
            if (!reached2) {
                uv2 += offset * QUALITY(i);
            }

            // If both sides have been reached, stop the exploration.
            if (reachedBoth) {
                break;
            }
        }
    }

    // Compute the distances to each extremity of the edge.
    float distance1 = isHorizontal ? (fragUV.x - uv1.x) : (fragUV.y - uv1.y);
    float distance2 = isHorizontal ? (uv2.x - fragUV.x) : (uv2.y - fragUV.y);

    // In which direction is the extremity of the edge closer ?
    bool isDirection1 = distance1 < distance2;
    float distanceFinal = min(distance1, distance2);

    // Length of the edge.
    float edgeThickness = (distance1 + distance2);

    // UV offset: read in the direction of the closest side of the edge.
    float pixelOffset = - distanceFinal / edgeThickness + 0.5;

    // Is the luma at center smaller than the local average ?
    bool isLumCenterSmaller = lumMid < lumLocalAverage;

    // If the luma at center is smaller than at its neighbour,
    // the delta luma at each end should be positive (same variation).
    // (in the direction of the closer side of the edge)
    bool correctVariation = ((isDirection1 ? lumEnd1 : lumEnd2) < 0.0)
        != isLumCenterSmaller;

    // If the luma variation is incorrect, do not offset.
    float finalOffset = correctVariation ? pixelOffset : 0.0;

    // Sub-pixel shifting
    // Full weighted average of the luma over the 3x3 neighborhood.
    float lumAverage = (1.0/12.0) * (2.0 * (lumBotTop + lumLeftRight)
        + lumLeftCorners + lumRightCorners);
    // Ratio of the delta between the global average and the center luma,
    // over the luma delta in the 3x3 neighborhood.
    float subPixelOffset1 = clamp(abs(lumAverage - lumMid) / lumDelta,
        0.0, 1.0);
    float subPixelOffset2 = (-2.0 * subPixelOffset1 + 3.0)
        * subPixelOffset1 * subPixelOffset1;
    // Compute a sub-pixel offset based on this delta.
    float subPixelOffsetFinal = subPixelOffset2 * subPixelOffset2
        * SUBPIXEL_QUALITY;

    // Pick the biggest of the two offsets.
    finalOffset = max(finalOffset, subPixelOffsetFinal);

    // Compute the final UV coordinates.
    vec2 finalUV = fragUV;
    if (isHorizontal) {
        finalUV.y += finalOffset * stepLength;
    } else {
        finalUV.x += finalOffset * stepLength;
    }

    // Read the color at the new UV coordinates, and use it.
    vec3 finalColor = texture(framebufferTexture, finalUV).rgb;
    outColor = vec4(finalColor, 1.0);
}