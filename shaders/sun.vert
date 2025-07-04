#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

// UBO pour les données de caméra
layout (std140) uniform CameraUBO {
    mat4 projection;
    mat4 view;
    vec3 viewPos;
};

// UBO pour les données de transformation
layout (std140) uniform TransformUBO {
    mat4 model;
    mat4 normalMatrix;
};

uniform float time;

out vec3 FragPos;
out vec3 Normal;
out float Time;

void main()
{
    Time = time;
    
    // Calcul des positions et normales
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(normalMatrix) * aNormal;
    
    gl_Position = projection * view * vec4(FragPos, 1.0);
}
