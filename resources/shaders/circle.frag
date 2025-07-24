#version 100

precision mediump float;

varying vec2 originalPos;
varying vec4 color;

uniform float screenScale;

float screenPxRange() {
    return screenScale;
}

void main() {
    float dist = length(originalPos);
    float edge_width = 0.025 / screenScale;
    float alpha = 1.0 - smoothstep(0.5 - edge_width, 0.5, dist);
    gl_FragColor = vec4(color.rgb, alpha * color.a);
}
