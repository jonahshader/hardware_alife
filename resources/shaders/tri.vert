#version 100

attribute vec3 aPos;
attribute vec2 aTexPos;

varying vec2 a_texPos;

void main() {
    gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);
}
