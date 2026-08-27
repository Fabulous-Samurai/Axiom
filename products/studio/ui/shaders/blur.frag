#version 150

uniform sampler2D source;
uniform float qt_Opacity;
uniform float blurRadius;
varying vec2 qt_TexCoord0;

void main() {
    vec4 color = vec4(0.0);
    float total = 0.0;

    for (float x = -blurRadius; x <= blurRadius; x += 1.0) {
        for (float y = -blurRadius; y <= blurRadius; y += 1.0) {
            vec2 offset = vec2(x, y) / textureSize(source, 0);
            float weight = exp(-(x*x + y*y) / (2.0 * blurRadius * blurRadius));
            color += texture2D(source, qt_TexCoord0 + offset) * weight;
            total += weight;
        }
    }

    gl_FragColor = (color / total) * qt_Opacity;
}
