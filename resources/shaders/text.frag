#version 100

precision mediump float;

varying vec2 texCoord;
varying vec4 color;

uniform sampler2D msdf;
uniform float screenScale;
uniform vec2 textureSize;

float median(float r, float g, float b) {
    return max(min(r, g), min(max(r, g), b));
}

float screenPxRange() {
    vec2 unitRange = vec2(3.0, 3.0) / textureSize;
    return max(0.5 * screenScale * (unitRange.x + unitRange.y), 1.0);
}


void main() {
    vec3 msd = texture2D(msdf, texCoord).rgb;
    float sd = median(msd.r, msd.g, msd.b);
    float screenPxDistance = screenPxRange() * (sd - 0.5) * 16.0;
    float opacity = clamp(screenPxDistance + 0.5, 0.0, 1.0);
    gl_FragColor = vec4(color.rgb, opacity * color.a);
}
