#ifndef SPHERE_H
#define SPHERE_H

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>
#include <string>
#include "Mesh.h"
#include "Shader.h"
#include "TextureLoader.h"

class Sphere {
public:
    Sphere(float radius = 1.0f, unsigned int sectors = 36, unsigned int stacks = 18);
    Sphere(const std::string &texturePath, float radius = 1.0f, unsigned int sectors = 36, unsigned int stacks = 18);
    ~Sphere();
    void Draw(const Shader &shader) const;
private:
    Mesh* pMesh;
    void generateSphere(std::vector<Vertex> &vertices, std::vector<unsigned int> &indices, float radius, unsigned int sectors, unsigned int stacks);
};

#endif
