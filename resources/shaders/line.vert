// line.vert
// uses sdf to draw a line
// quad should be just big enough to contain the line
#version 100

attribute vec2 aPos;
attribute vec2 aTexCoord;
attribute float aLength; // start is always 0,0
attribute float aRadius;
attribute vec4 aColor;

varying vec2 texCoord;
varying float lineLength;
varying float radius;
varying vec4 color;

uniform mat4 transform;

void main() {
    texCoord = aTexCoord;
    lineLength = aLength;
    radius = aRadius;
    color = aColor;
    gl_Position = transform * vec4(aPos, 0.0, 1.0);
}
