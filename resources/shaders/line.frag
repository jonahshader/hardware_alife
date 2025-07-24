#version 100

precision mediump float;

varying vec2 texCoord;
varying float lineLength;
varying float radius;
varying vec4 color;

uniform float screenScale;

float screenPxRange(vec2 pos) {
    return screenScale;
}

float sdVerticalCapsule( vec2 p, float h, float r )
{
    p.y -= clamp( p.y, 0.0, h );
    return length(p) - r;
}

void main() {
    float d = -sdVerticalCapsule(texCoord, lineLength, radius);
    float screenPxDistance = screenPxRange(texCoord) * d;
    float opacity = clamp(screenPxDistance + 0.5, 0.0, 1.0);
    gl_FragColor = vec4(color.rgb, opacity * color.a);
}
