#version 100

precision mediump float;

varying vec2 originalPos;
varying vec2 size;
varying vec4 color;
varying float radius;

uniform float screenScale;

float screenPxRange(vec2 pos) {
    return screenScale;
}

float box(vec2 position, vec2 halfSize, float cornerRadius) {
    position = abs(position) - halfSize + cornerRadius;
    return length(max(position, 0.0)) + min(max(position.x, position.y), 0.0) - cornerRadius;
}

void main() {
    vec2 pos = originalPos * size;
    float d = -box(pos, size/2.0, radius);
    float screenPxDistance = screenPxRange(pos) * (d - 0.5);
    float opacity = clamp(screenPxDistance + 0.5, 0.0, 1.0);
    gl_FragColor = vec4(color.rgb, opacity * color.a);
}
