#version 450
/**
 * @file mantis_nav.frag
 * @brief High-intensity fragment shader for AOR (Area of Research) visualization.
 */

layout(location = 0) in vec4 fragColor;
layout(location = 0) out vec4 outColor;

void main() {
    // [MANDATORY PATH]: No expensive Gaussian blur or glassmorphism.
    // Pure information density via high-contrast spectral coloring.
    outColor = fragColor;
}
