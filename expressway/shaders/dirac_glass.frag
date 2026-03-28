#version 450

layout(location = 0) in vec2 fragTexCoord;
layout(location = 1) in float vTime;

layout(binding = 1) uniform sampler2D texSampler; // Background sampler for blur

layout(location = 0) out vec4 outColor;

// Dirac Palette
const vec3 DIRAC_INDIGO = vec3(0.06, 0.09, 0.16); // #0F172A
const vec3 DIRAC_CYAN = vec3(0.49, 0.83, 0.99);   // #7DD3FC

float random(vec2 co) {
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

void main() {
    // 1. Base Glass Tint (Deep Indigo with transparency)
    vec4 glassBase = vec4(DIRAC_INDIGO, 0.85);

    // 2. Sample Background with slight offset for "refraction"
    // In Phase 7, this will be a real blur pass. For now, we simulate.
    vec3 background = texture(texSampler, fragTexCoord).rgb;
    
    // 3. Dirac Accent Glow (Edge highlighting)
    float edge = smoothstep(0.0, 0.05, fragTexCoord.x) * (1.0 - smoothstep(0.95, 1.0, fragTexCoord.x));
    edge *= smoothstep(0.0, 0.05, fragTexCoord.y) * (1.0 - smoothstep(0.95, 1.0, fragTexCoord.y));
    
    vec3 finalColor = mix(DIRAC_INDIGO * 0.8, background, 0.3);
    
    // 4. Neon Edge Polish
    if (edge < 0.1) {
        finalColor = mix(finalColor, DIRAC_CYAN, 0.4);
    }

    // 5. Industrial Grain (Noise)
    float noise = (random(fragTexCoord + vTime * 0.01) - 0.5) * 0.05;
    finalColor += noise;

    outColor = vec4(finalColor, 0.9);
}
