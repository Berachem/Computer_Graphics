#include "Sphere.h"
#include <cmath>
#include <iostream>

// Constructeur sans texture
Sphere::Sphere(float radius, unsigned int sectors, unsigned int stacks) {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures; // Pas de textures pour la sphère blanche

    // Générer la géométrie de la sphère
    generateSphere(vertices, indices, radius, sectors, stacks);

    // Créer le mesh dynamiquement
    pMesh = new Mesh(vertices, indices, textures);
}

// Constructeur avec texture
Sphere::Sphere(const std::string &texturePath, float radius, unsigned int sectors, unsigned int stacks) {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;

    // Générer la géométrie de la sphère
    generateSphere(vertices, indices, radius, sectors, stacks);

    // Charger la texture
    Texture texture;
    texture.id = loadTexture(texturePath.c_str());
    texture.type = "texture_diffuse";
    texture.path = texturePath;
    textures.push_back(texture);

    // Créer le mesh dynamiquement
    pMesh = new Mesh(vertices, indices, textures);
}

// Destructeur pour libérer la mémoire
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

// Méthode pour générer la géométrie de la sphère
void Sphere::generateSphere(std::vector<Vertex> &vertices, std::vector<unsigned int> &indices,
                           float radius, unsigned int sectors, unsigned int stacks) {
    // Générer les vertices
    float sectorStep = 2 * glm::pi<float>() / sectors;
    float stackStep = glm::pi<float>() / stacks;

    for (unsigned int i = 0; i <= stacks; ++i) {
        float stackAngle = glm::pi<float>() / 2 - i * stackStep; // Angle de -pi/2 à pi/2
        float xy = radius * cosf(stackAngle);
        float z = radius * sinf(stackAngle);

        // Ajouter (sectors+1) vertices par stack
        // Les derniers vertices de chaque stack sont les mêmes que les premiers
        for (unsigned int j = 0; j <= sectors; ++j) {
            float sectorAngle = j * sectorStep; // Angle de 0 à 2pi

            // Position du vertex
            float x = xy * cosf(sectorAngle);
            float y = xy * sinf(sectorAngle);

            // Créer le vertex
            Vertex vertex;
            vertex.Position = glm::vec3(x, y, z);

            // Normale (normalisée)
            vertex.Normal = glm::normalize(vertex.Position);

            // Coordonnées de texture
            vertex.TexCoords = glm::vec2((float)j / sectors, (float)i / stacks);

            vertices.push_back(vertex);
        }
    }

    // Générer les indices
    for (unsigned int i = 0; i < stacks; ++i) {
        unsigned int k1 = i * (sectors + 1);
        unsigned int k2 = k1 + sectors + 1;

        for (unsigned int j = 0; j < sectors; ++j, ++k1, ++k2) {
            // 2 triangles par secteur

            // Triangle 1
            if (i != 0) {
                indices.push_back(k1);
                indices.push_back(k2);
                indices.push_back(k1 + 1);
            }

            // Triangle 2
            if (i != (stacks - 1)) {
                indices.push_back(k1 + 1);
                indices.push_back(k2);
                indices.push_back(k2 + 1);
            }
        }
    }
}

