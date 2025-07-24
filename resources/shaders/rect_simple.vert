#version 100

attribute vec2 aPos;
attribute vec2 aOffset;
attribute vec2 aSize;
attribute vec4 aColor;

varying vec4 color;

uniform mat4 transform;

void main() {
    color = aColor;
    gl_Position = transform * vec4(aPos * aSize + aOffset, 0.0, 1.0);
}
