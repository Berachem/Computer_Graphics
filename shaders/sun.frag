#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in float Time;

// Constantes pour le bruit de Perlin - augmenter les octaves et lacunarity
const int octaves = 6;        // Augmente pour plus de détails
const float persistence = 0.6; // Légèrement augmenté pour renforcer les détails
const float lacunarity = 2.5;  // Augmente pour des taches plus petites

// Fonction pour permuter les valeurs (hash)
float hash(float n) {
    return fract(sin(n) * 43758.5453);
}

// Bruit 3D basé sur la position dans l'espace
float noise3D(vec3 p) {
    vec3 i = floor(p);
    vec3 f = fract(p);
    
    // Lissage cubique
    f = f * f * (3.0 - 2.0 * f);
    
    // Points de la grille
    float n000 = hash(dot(i, vec3(13.46, 41.74, 289.37)));
    float n001 = hash(dot(i + vec3(0,0,1), vec3(13.46, 41.74, 289.37)));
    float n010 = hash(dot(i + vec3(0,1,0), vec3(13.46, 41.74, 289.37)));
    float n011 = hash(dot(i + vec3(0,1,1), vec3(13.46, 41.74, 289.37)));
    float n100 = hash(dot(i + vec3(1,0,0), vec3(13.46, 41.74, 289.37)));
    float n101 = hash(dot(i + vec3(1,0,1), vec3(13.46, 41.74, 289.37)));
    float n110 = hash(dot(i + vec3(1,1,0), vec3(13.46, 41.74, 289.37)));
    float n111 = hash(dot(i + vec3(1,1,1), vec3(13.46, 41.74, 289.37)));
    
    // Interpolation trilinéaire
    return mix(
        mix(
            mix(n000, n100, f.x),
            mix(n010, n110, f.x),
            f.y
        ),
        mix(
            mix(n001, n101, f.x),
            mix(n011, n111, f.x),
            f.y
        ),
        f.z
    );
}

// FBM 3D
float fbm3D(vec3 p) {
    float value = 0.0;
    float amplitude = 1.0;
    float frequency = 1.0;
    
    for(int i = 0; i < octaves; i++) {
        value += amplitude * noise3D(p * frequency);
        frequency *= lacunarity;
        amplitude *= persistence;
    }
    
    return value;
}

void main() {
    vec3 p = normalize(FragPos);
    
    // Adapter l'échelle pour le bruit 3D - augmenter pour des détails plus fins
    p *= 4.0; // Valeur doublée pour des motifs plus petits et plus nombreux
    
    // Animation avec le temps
    float t = Time * 0.1;
    
    // Utiliser du bruit 3D pour éviter les coutures - ajout d'une couche supplémentaire
    float n1 = fbm3D(p + vec3(t * 0.1));
    float n2 = fbm3D(p * 2.0 + vec3(-t * 0.15, t * 0.05, t * 0.1));
    // Nouvelle couche de bruit à plus petite échelle
    float n3 = fbm3D(p * 5.0 + vec3(t * 0.25, -t * 0.2, t * 0.15));
    
    // Mélange de bruits pour créer des motifs complexes - intégrer la nouvelle couche
    float finalNoise = n1 * 0.5 + n2 * 0.3 + n3 * 0.2;
    
    // Couleurs de base pour le soleil (version plus foncée)
    vec3 brightColor = vec3(0.95, 0.7, 0.3);    // Orange-doré moins brillant
    vec3 midColor = vec3(0.8, 0.4, 0.05);       // Orange plus foncé
    vec3 darkColor = vec3(0.6, 0.2, 0.0);      // Rouge-brun foncé
    
    // Mélange les couleurs en fonction du bruit
    vec3 color;
    if (finalNoise > 0.65) {
        color = mix(brightColor, midColor, (finalNoise - 0.65) * 2.85);
    } else {
        color = mix(darkColor, midColor, finalNoise * 1.54);
    }
    
    // Effet de pulsation globale
    float pulse = 0.05 * sin(Time * 0.5);
    color += pulse;
    
    // Ajouter un effet de "glow" sur les bords
    vec3 viewDir = normalize(-FragPos);
    float rim = 1.0 - max(dot(viewDir, normalize(Normal)), 0.0);
    rim = pow(rim, 2.0);
    color += rim * 0.3 * brightColor;
    
    // Résultat final avec luminosité élevée (c'est un soleil!)
    FragColor = vec4(color, 1.0);
}
