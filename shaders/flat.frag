#version 330 core
out vec4 FragColor;
uniform vec3  objectColor;
uniform float alpha = 1.0;
void main() {
    FragColor = vec4(objectColor, alpha);
}
