#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in float Time;

// Constantes pour le bruit de Perlin
const int octaves = 3;        
const float persistence = 0.6;
const float lacunarity = 2.0;

//fonction pour permuter les valeurs (hash)
float hash(float n) {
    return fract(sin(n) * 43758.5453);
}

//bruit 3D basé sur la position dans l'espace
float noise3D(vec3 p) {
    vec3 i = floor(p);
    vec3 f = fract(p);
    
    //lissage cubique
    f = f * f * (3.0 - 2.0 * f);
    
    //points de la grille
    float n000 = hash(dot(i, vec3(13.46, 41.74, 289.37)));
    float n001 = hash(dot(i + vec3(0,0,1), vec3(13.46, 41.74, 289.37)));
    float n010 = hash(dot(i + vec3(0,1,0), vec3(13.46, 41.74, 289.37)));
    float n011 = hash(dot(i + vec3(0,1,1), vec3(13.46, 41.74, 289.37)));
    float n100 = hash(dot(i + vec3(1,0,0), vec3(13.46, 41.74, 289.37)));
    float n101 = hash(dot(i + vec3(1,0,1), vec3(13.46, 41.74, 289.37)));
    float n110 = hash(dot(i + vec3(1,1,0), vec3(13.46, 41.74, 289.37)));
    float n111 = hash(dot(i + vec3(1,1,1), vec3(13.46, 41.74, 289.37)));
    
    //interpolation trilinéaire
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
    //normalisation de la position du fragment
    vec3 p = normalize(FragPos);

    //couleurs pour le soleil
    vec3 brightColor = vec3(0.95, 0.7, 0.3);    //orange-doré
    vec3 midColor = vec3(0.8, 0.4, 0.05);       //orange
    vec3 darkColor = vec3(0.6, 0.2, 0.0);       //rouge-brun
    
    
    //adapter l'échelle pour le bruit 3D
    p *= 3.0;
    
    //animation avec le temps
    float t = Time * 0.1;
    
    //utilisation du bruit 3D  en plusieurs couches
    float n1 = fbm3D(p + vec3(t * 0.1));
    float n2 = fbm3D(p * 2.0 + vec3(-t * 0.15, t * 0.05, t * 0.1));
    float n3 = fbm3D(p * 5.0 + vec3(t * 0.25, -t * 0.2, t * 0.15));
    
    //mélange de bruits pour créer des motifs complexes
    float finalNoise = n1 * 0.5 + n2 * 0.3 + n3 * 0.2; 

    //mélange les couleurs en fonction du bruit
    vec3 color = mix(brightColor, midColor, (finalNoise - 0.65) * 2.85);
    
    //effet de pulsation (lent)
    float pulse = 0.07 * sin(Time * 0.7);
    color += pulse;
    
    //rendu final
    FragColor = vec4(color, 1.0);
}
