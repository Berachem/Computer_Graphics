#version 330 core
out vec4 FragColor;

uniform vec3 objectColor = vec3(0.8, 0.3, 0.2); // Valeur par défaut, peut être remplacée par le programme

void main()
{
    // On utilise directement la couleur fournie
    FragColor = vec4(objectColor, 1.0);
}
