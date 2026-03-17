#version 150

uniform float qt_Opacity;
uniform float strength;
varying vec2 qt_TexCoord0;

float random(vec2 st) {
    return fract(sin(dot(st.xy, vec2(12.9898, 78.233))) * 43758.5453123);
}

void main() {
    float noise = random(qt_TexCoord0) * strength;
    gl_FragColor = vec4(noise, noise, noise, 0.03) * qt_Opacity;
}