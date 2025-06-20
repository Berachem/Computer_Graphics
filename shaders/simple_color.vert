#version 330 core

layout (location = 0) in vec3 aPos;

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

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
