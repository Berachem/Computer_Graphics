#ifndef MODEL_H
#define MODEL_H

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <string>
#include <vector>
#include "Mesh.h"
#include "Shader.h"
#include "tiny_obj_loader.h"

class Model
{
public:
    // Données
    std::vector<Mesh> meshes;
    std::string directory;

    // Constructeur
    Model(const std::string &path);

    // Afficher le modèle
    void Draw(const Shader &shader) const;

private:
    void loadModel(const std::string &path);
    Mesh processMesh(const tinyobj::attrib_t &attrib, const tinyobj::shape_t &shape, const std::vector<tinyobj::material_t> &materials);
};

#endif