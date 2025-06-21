#include "Sphere.h"
#include <cmath>

Sphere::Sphere(float radius, unsigned int sectors, unsigned int stacks) {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;
    generateSphere(vertices, indices, radius, sectors, stacks);
    pMesh = new Mesh(vertices, indices, textures);
}

Sphere::Sphere(const std::string &texturePath, float radius, unsigned int sectors, unsigned int stacks) {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;
    generateSphere(vertices, indices, radius, sectors, stacks);
    Texture texture;
    texture.id = loadTexture(texturePath.c_str());
    texture.type = "texture_diffuse";
    texture.path = texturePath;
    textures.push_back(texture);
    pMesh = new Mesh(vertices, indices, textures);
}

Sphere::~Sphere() {
    if (pMesh) {
        delete pMesh;
        pMesh = nullptr;
    }
}

void Sphere::Draw(const Shader &shader) const {
    if (pMesh) {
        pMesh->Draw(shader);
    }
}

void Sphere::generateSphere(std::vector<Vertex> &vertices, std::vector<unsigned int> &indices, float radius, unsigned int sectors, unsigned int stacks) {
    float sectorStep = 2 * glm::pi<float>() / sectors;
    float stackStep = glm::pi<float>() / stacks;
    for (unsigned int i = 0; i <= stacks; ++i) {
        float stackAngle = glm::pi<float>() / 2 - i * stackStep;
        float xy = radius * cosf(stackAngle);
        float z = radius * sinf(stackAngle);
        for (unsigned int j = 0; j <= sectors; ++j) {
            float sectorAngle = j * sectorStep;
            Vertex vertex;
            vertex.Position.x = xy * cosf(sectorAngle);
            vertex.Position.y = xy * sinf(sectorAngle);
            vertex.Position.z = z;
            vertex.Normal = glm::normalize(vertex.Position);
            vertex.TexCoords.x = (float)j / sectors;
            vertex.TexCoords.y = (float)i / stacks;
            vertices.push_back(vertex);
        }
    }
    for (unsigned int i = 0; i < stacks; ++i) {
        for (unsigned int j = 0; j < sectors; ++j) {
            unsigned int first = i * (sectors + 1) + j;
            unsigned int second = first + sectors + 1;
            indices.push_back(first);
            indices.push_back(second);
            indices.push_back(first + 1);
            indices.push_back(second);
            indices.push_back(second + 1);
            indices.push_back(first + 1);
        }
    }
}

