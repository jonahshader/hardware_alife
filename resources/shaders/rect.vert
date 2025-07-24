#version 100

attribute vec2 aPos;
attribute vec2 aOffset;
attribute vec2 aSize;
attribute float aRadius;
attribute vec4 aColor;

varying vec2 originalPos;
varying vec2 size;
varying vec4 color;
varying float radius;

uniform mat4 transform;

void main() {
    originalPos = aPos;
    color = aColor;
    radius = aRadius;
    size = aSize;
    gl_Position = transform * vec4(aPos * aSize + aOffset, 0.0, 1.0);
}
