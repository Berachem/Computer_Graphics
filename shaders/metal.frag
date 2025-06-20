#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec3 ViewDir;

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

uniform vec3 metalColor = vec3(0.9, 0.9, 0.95);  

void main()
{
    // Normalisation des vecteurs
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    vec3 viewDir = normalize(ViewDir);
    
    // Lumière ambiante
    vec3 ambient = ambientStrength * ambientColor * lightColor;

    // Lumière diffuse
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor * 0.35;

    // Couleur du métal en fonction des lumières
    vec3 metalBase = metalColor * (ambient + diffuse * 1.3);
    
    // Flare spéculaire métallique
    float reflectDot = max(dot(reflect(-lightDir, norm), viewDir), 0.0);
    float flareIntensity = pow(reflectDot, shininess);
    
    // Assemblage final avec les flares 
    vec3 lighting = metalBase + (flareIntensity * specularStrength * lightColor);
    FragColor = vec4(lighting, 1.0);
}
