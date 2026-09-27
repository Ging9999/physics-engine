#version 330 core

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;
in vec4 FragPosLightSpace;

out vec4 FragColor;

uniform vec3 objectColor;
uniform vec3 lightColor;
uniform vec3 lightPos;
uniform vec3 viewPos;
uniform int  lightingMode;

// Texture
uniform sampler2D textureSampler;
uniform bool      useTexture;

// Material
uniform vec3  material_ambient;
uniform vec3  material_diffuse;
uniform vec3  material_specular;
uniform float material_shininess;

// Shadow
uniform sampler2D shadowMap;
uniform bool      shadowsEnabled;
uniform mat4      lightSpaceMatrix; // used only to satisfy the vert output

float calculateShadow(vec4 fragPosLS) {
    vec3 projCoords = fragPosLS.xyz / fragPosLS.w;
    projCoords = projCoords * 0.5 + 0.5;
    if (projCoords.z > 1.0) return 0.0;
    float bias = max(0.005 * (1.0 - dot(normalize(Normal), normalize(lightPos - FragPos))), 0.001);
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for (int x = -1; x <= 1; x++) {
        for (int y = -1; y <= 1; y++) {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += (projCoords.z - bias) > pcfDepth ? 1.0 : 0.0;
        }
    }
    return shadow / 9.0;
}

void main() {
    vec3 baseColor = useTexture ? texture(textureSampler, TexCoord).rgb : objectColor;

    vec3 norm     = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    vec3 viewDir  = normalize(viewPos - FragPos);
    vec3 reflDir  = reflect(-lightDir, norm);

    vec3 ambient  = material_ambient * lightColor;
    float diff    = max(dot(norm, lightDir), 0.0);
    vec3 diffuse  = material_diffuse * diff * lightColor;
    float spec    = pow(max(dot(viewDir, reflDir), 0.0), max(material_shininess, 1.0));
    vec3 specular = material_specular * spec * lightColor;

    float shadow  = shadowsEnabled ? calculateShadow(FragPosLightSpace) : 0.0;

    vec3 result;
    if (lightingMode == 1) {
        result = ambient * baseColor;
    } else if (lightingMode == 2) {
        result = (ambient + (1.0 - shadow) * diffuse) * baseColor;
    } else if (lightingMode == 3) {
        result = (ambient + (1.0 - shadow) * specular) * baseColor;
    } else {
        result = (ambient + (1.0 - shadow) * (diffuse + specular)) * baseColor;
    }

    FragColor = vec4(result, 1.0);
}
