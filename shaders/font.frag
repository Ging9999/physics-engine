#version 330 core
in vec2 TexCoord;
out vec4 FragColor;
uniform sampler2D fontAtlas;
uniform vec3 textColor;
void main() {
    float a = texture(fontAtlas, TexCoord).r;
    if (a < 0.1) discard;
    FragColor = vec4(textColor, a);
}
