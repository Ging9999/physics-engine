#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aColor;
layout(location = 2) in float aAlpha;
out vec4 particleColor;
uniform mat4 view;
uniform mat4 projection;
void main() {
    gl_Position  = projection * view * vec4(aPos, 1.0);
    gl_PointSize = 4.0;
    particleColor = vec4(aColor, aAlpha);
}
