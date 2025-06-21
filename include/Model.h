#ifndef MODEL_H
#define MODEL_H

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <string>
#include <vector>
#include "Mesh.h"
#include "Shader.h"
#include "tiny_obj_loader.h"

// Modèle 3D chargé depuis un fichier OBJ
class Model {
  public:
    std::vector<Mesh> meshes;
    std::string directory;

    Model(const std::string &path);

    void Draw(const Shader &shader) const;

  private:
    void loadModel(const std::string &path);
    Mesh processMesh(const tinyobj::attrib_t &attrib, const tinyobj::shape_t &shape, const std::vector<tinyobj::material_t> &materials);
};

#endif