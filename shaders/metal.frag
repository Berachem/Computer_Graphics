#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec3 ViewDir;

uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 lightColor = vec3(1.0);

// Paramètres spécifiques pour l'effet métallique - ajustés pour plus de brillance
uniform vec3 metalColor = vec3(0.9, 0.9, 0.95);  // Gris plus clair et brillant
uniform float metallic = 1.0;                    // 100% métallique
uniform float roughness = 0.05;                  // Beaucoup moins rugueux (plus brillant)
uniform float metalFresnelFactor = 0.98;         // Effet Fresnel renforcé

void main()
{
    // Normalisation des vecteurs
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    vec3 viewDir = normalize(ViewDir);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    
    // Lumière ambiante maintenue basse pour contraste
    float ambientStrength = 0.12;
    vec3 ambient = ambientStrength * lightColor;
    
    // Lumière diffuse standard
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor * 0.35;
    
    // FLARE PRINCIPAL - Effet intense et localisé basé sur la réflexion parfaite
    float reflectDot = max(dot(reflect(-lightDir, norm), viewDir), 0.0);
    float flareIntensity = pow(reflectDot, 32.0) * 8.0; // Plus large mais toujours intense
    
    // FLARE SECONDAIRE - Effet Blinn-Phong amplifié pour des reflets plus larges
    float specFactor = pow(max(dot(norm, halfwayDir), 0.0), 64.0) * 1.5;
    
    // SUPPRESSION de l'effet anisotropique qui causait probablement les lignes horizontales
    // Remplacement par un effet spéculaire secondaire plus doux
    float softSpec = pow(max(dot(norm, halfwayDir), 0.0), 16.0) * 0.3;
    
    // EFFET DE HALO au bord de l'objet
    float rim = pow(1.0 - max(dot(viewDir, norm), 0.0), 3.0);
    float rimLight = rim * 0.6 * max(dot(norm, lightDir), 0.0);
    
    // Sparkles modifiés pour éviter les alignements qui peuvent causer des lignes
    vec3 sparklePos = FragPos * 0.15; // Échelle encore réduite
    // Utilisation de vecteurs non alignés pour éviter les motifs linéaires
    float sparkleNoise = fract(sin(dot(sparklePos, vec3(12.9898, 78.233, 45.164))) * 43758.5453);
    float sparkleThreshold = 0.985; // Seuil encore augmenté = encore moins de points
    float sparkleIntensity = (sparkleNoise > sparkleThreshold) ? 1.2 : 0.0;
    float sparkle = sparkleIntensity * pow(max(dot(norm, lightDir), 0.0), 4.0) * 
                   smoothstep(0.6, 0.95, reflectDot); // Davantage concentré sur les zones de reflet
    
    // Base métallique plus lisse et légèrement plus foncée pour meilleur contraste
    vec3 metalBase = metalColor * (ambient + diffuse * 1.3);
    vec3 flareColor = lightColor; // Couleur de flare neutre
    
    // Assemblage final avec les flares mais sans l'effet anisotropique problématique
    vec3 lighting = metalBase + 
                   (flareIntensity * flareColor) +  // Flare principal
                   (specFactor * flareColor) +      // Flare secondaire intense
                   (softSpec * flareColor) +        // Spéculaire doux remplaçant l'anisotropique
                   (rimLight * flareColor) +        // Halo au bord
                   (sparkle * flareColor);          // Sparkles réduits
    
    // Lissage plus important pour éliminer toute trace de motifs linéaires
    lighting = mix(lighting, metalBase, 0.25);
    
    // Compression tonale avec des valeurs ajustées pour un rendu plus naturel
    lighting = lighting / (lighting + vec3(0.75));
    
    // Contraste doux et teinte légère
    lighting = pow(lighting, vec3(0.85));
    vec3 tint = vec3(1.02, 1.0, 1.03);
    vec3 result = lighting * tint;
    
    FragColor = vec4(result, 1.0);
}
