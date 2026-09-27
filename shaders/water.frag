#version 330 core

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

out vec4 FragColor;

uniform vec3  lightPos;
uniform vec3  viewPos;
uniform float time;

void main() {
    // Micro-shimmer: perturb the geometry normal slightly over time
    float shimmer = sin(TexCoord.x * 40.0 + time * 2.5) *
                    cos(TexCoord.y * 40.0 + time * 1.9) * 0.03;
    vec3 norm = normalize(Normal + vec3(shimmer, 0.0, shimmer));

    vec3 lightDir = normalize(lightPos - FragPos);
    vec3 viewDir  = normalize(viewPos  - FragPos);
    vec3 halfway  = normalize(lightDir + viewDir);

    // Fresnel: surface looks more opaque / reflective at grazing angles
    float cosTheta = max(dot(norm, viewDir), 0.0);
    float fresnel  = 0.08 + 0.72 * pow(1.0 - cosTheta, 4.0);

    // Lighting
    float diff = max(dot(norm, lightDir), 0.15);
    float spec = pow(max(dot(norm, halfway), 0.0), 256.0);

    // Color: deep blue at low normals, lighter turquoise at the peaks
    vec3 deep    = vec3(0.00, 0.08, 0.28);
    vec3 surface = vec3(0.05, 0.38, 0.68);
    vec3 water   = mix(deep, surface, clamp(norm.y * 1.2, 0.0, 1.0));

    vec3 ambient  = water * 0.22;
    vec3 diffuse  = water * diff * 0.72;
    vec3 specular = vec3(1.0, 0.97, 0.93) * spec * 1.1;

    vec3  result = ambient + diffuse + specular;
    float alpha  = mix(0.48, 0.90, fresnel);

    FragColor = vec4(result, alpha);
}
