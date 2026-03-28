#version 450

layout(location = 0) in vec2 fragTexCoord;
layout(location = 1) in float vTime;

layout(location = 0) out vec4 outColor;

const vec3 DIRAC_CYAN = vec3(0.49, 0.83, 0.99);

void main() {
    // Distance from center for circular nodes
    vec2 center = vec2(0.5, 0.5);
    float dist = distance(fragTexCoord, center);
    
    // Smooth circle boundary
    float circle = 1.0 - smoothstep(0.4, 0.45, dist);
    
    // Dynamic Pulse effect based on time (Pluto Swarm Activity)
    float pulse = 0.8 + 0.2 * sin(vTime * 5.0);
    
    // Inner Glow
    float glow = exp(-5.0 * dist) * pulse;
    
    vec3 color = DIRAC_CYAN * glow;
    
    // Outer Ring
    float ringWidth = 0.02;
    float ring = smoothstep(0.46, 0.46 + ringWidth, dist) * (1.0 - smoothstep(0.48 + ringWidth, 0.48 + 2.0*ringWidth, dist));
    
    color += DIRAC_CYAN * ring * 2.0;

    outColor = vec4(color, circle);
}
