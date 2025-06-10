#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 FragPos;
out vec3 Normal;
out vec3 ViewDir;

void main()
{
    FragPos = vec3(model * vec4(aPos, 1.0));
    //calcul des normales avec la matrice inverse transposée
    Normal = mat3(transpose(inverse(model))) * aNormal;
    
    //calculer le vecteur de vue pour les effets
    vec3 viewPos = vec3(inverse(view)[3]);
    ViewDir = normalize(viewPos - FragPos);
    
    gl_Position = projection * view * vec4(FragPos, 1.0);
}
