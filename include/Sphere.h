#ifndef SPHERE_H
#define SPHERE_H

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <vector>
#include <string>
#include "Mesh.h"
#include "Shader.h"
#include "TextureLoader.h"

class Sphere {
public:
    // Constructeur avec paramètres pour la résolution de la sphère
    Sphere(float radius = 1.0f, unsigned int sectors = 36, unsigned int stacks = 18);

    // Constructeur avec texture
    Sphere(const std::string &texturePath, float radius = 1.0f, unsigned int sectors = 36, unsigned int stacks = 18);

    // Destructeur pour libérer la mémoire
    ~Sphere();

    // Dessiner la sphère
    void Draw(const Shader &shader) const;

private:
    // Pointeur vers le mesh de la sphère (pour éviter le problème de constructeur par défaut)
    Mesh* pMesh;

    // Méthode pour générer la géométrie de la sphère
    void generateSphere(std::vector<Vertex> &vertices, std::vector<unsigned int> &indices,
                        float radius, unsigned int sectors, unsigned int stacks);
};

#endif
