#include "Model.h"
#include "tiny_obj_loader.h"

#include <iostream>

Model::Model(const std::string &path) { loadModel(path); }

void Model::Draw(const Shader &shader) const {
    for (unsigned int i = 0; i < meshes.size(); i++)
        meshes[i].Draw(shader);
}

void Model::loadModel(const std::string &path) {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    directory = path.substr(0, path.find_last_of('/'));

    bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str(), directory.c_str(), true);

    if (!ret) {
        std::cerr << "Erreur : échec de chargement du modèle " << path << std::endl;
        return;
    }

    for (size_t s = 0; s < shapes.size(); s++) {
        meshes.push_back(processMesh(attrib, shapes[s], materials));
    }
}

Mesh Model::processMesh(const tinyobj::attrib_t &attrib, const tinyobj::shape_t &shape, const std::vector<tinyobj::material_t> &materials) {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;

    for (size_t f = 0; f < shape.mesh.indices.size(); f++) {
        tinyobj::index_t idx = shape.mesh.indices[f];

        Vertex vertex;

        vertex.Position = glm::vec3(
            attrib.vertices[3 * idx.vertex_index + 0],
            attrib.vertices[3 * idx.vertex_index + 1],
            attrib.vertices[3 * idx.vertex_index + 2]);

        if (idx.normal_index >= 0) {
            vertex.Normal = glm::vec3(
                attrib.normals[3 * idx.normal_index + 0],
                attrib.normals[3 * idx.normal_index + 1],
                attrib.normals[3 * idx.normal_index + 2]);
        } else {
            vertex.Normal = glm::vec3(0.0f, 0.0f, 0.0f); // Valeur par défaut
        }

        if (idx.texcoord_index >= 0) {
            vertex.TexCoords = glm::vec2(
                attrib.texcoords[2 * idx.texcoord_index + 0],
                attrib.texcoords[2 * idx.texcoord_index + 1]);
        } else {
            vertex.TexCoords = glm::vec2(0.0f, 0.0f);
        }

        vertices.push_back(vertex);
        indices.push_back((unsigned int)f);
    }

    return Mesh(vertices, indices, textures);
}
