#version 150

uniform sampler2D source;
uniform float qt_Opacity;
uniform vec4 glowColor;
uniform float intensity;
varying vec2 qt_TexCoord0;

void main() {
    vec4 texColor = texture2D(source, qt_TexCoord0);
    float luminance = dot(texColor.rgb, vec3(0.299, 0.587, 0.114));
    vec4 glow = glowColor * luminance * intensity;
    gl_FragColor = (texColor + glow) * qt_Opacity;
}
