#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;

// UBO pour les données de caméra
layout (std140) uniform CameraUBO {
    mat4 projection;
    mat4 view;
    vec3 viewPos;
};

// UBO pour les données d'éclairage
layout (std140) uniform LightingUBO {
    vec3 lightPos;
    vec3 lightColor;
    vec3 ambientColor;
    float ambientStrength;
    float specularStrength;
    float shininess;
};

uniform vec3 objectColor = vec3(0.8, 0.2, 0.6);

void main()
{
    // Ambient
    vec3 ambient = ambientStrength * ambientColor * lightColor;

    // Diffuse (Lambert)
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    // Pas de composante spéculaire dans Lambert
    vec3 result = (ambient + diffuse) * objectColor;
    FragColor = vec4(result, 1.0);
}
