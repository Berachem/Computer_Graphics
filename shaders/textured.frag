#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

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

uniform sampler2D texture_diffuse1;

void main()
{
    // Texture
    vec4 texColor = texture(texture_diffuse1, TexCoords);
    
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
    
    // Résultat final
    vec3 result = (ambient + diffuse + specular) * texColor.rgb;
    FragColor = vec4(result, 1.0);
}
