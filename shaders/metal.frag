#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec3 ViewDir;

uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 lightColor = vec3(1.0);

uniform vec3 metalColor = vec3(0.9, 0.9, 0.95);  

void main()
{
    //normalisation des vecteurs
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    vec3 viewDir = normalize(ViewDir);
    
    //lumière ambiante
    float ambientStrength = 0.12;
    vec3 ambient = ambientStrength * lightColor;

    //lumière diffuse
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor * 0.35;

    //couleur du metal en fonction des lumières
    vec3 metalBase = metalColor * (ambient + diffuse * 1.3);
    
    //flare
    float reflectDot = max(dot(reflect(-lightDir, norm), viewDir), 0.0);
    float flareIntensity = pow(reflectDot, 32.0);
    
    //assemblage final avec les flares 
    vec3 lighting = metalBase + (flareIntensity * lightColor);
    FragColor = vec4(lighting, 1.0);
}
