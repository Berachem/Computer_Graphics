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

uniform vec3 objectColor = vec3(1.0, 0.5, 0.3);

void main()
{
    // Ambient
    vec3 ambient = ambientStrength * ambientColor * lightColor;

    // Diffuse
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    // Specular
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    vec3 specular = specularStrength * spec * lightColor;

    vec3 result = (ambient + diffuse + specular) * objectColor;
    FragColor = vec4(result, 1.0);
}
