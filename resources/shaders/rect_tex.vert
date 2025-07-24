#version 100

attribute vec2 aPos;
attribute vec3 aColor;
attribute vec2 aTexCoord;

varying vec3 ourColor;
varying vec2 TexCoord;

uniform mat4 transform;

void main()
{
  gl_Position = transform * vec4(aPos, 0.0, 1.0);
  ourColor = aColor;
  TexCoord = aTexCoord;
}
