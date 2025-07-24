#version 100

attribute vec2 aPos;
attribute vec2 aTexCoord;
attribute vec4 aColor;

uniform mat4 transform;

varying vec2 texCoord;
varying vec4 color;

void main() {
    texCoord = aTexCoord;
    color = aColor;
    gl_Position = transform * vec4(aPos, 0.0, 1.0);
}
