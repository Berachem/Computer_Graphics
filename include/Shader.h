#ifndef SHADER_H
#define SHADER_H

#include <string>
#include <glm/glm.hpp>

class Shader
{
public:
    unsigned int ID;

    // Constructeur : lit et construit le shader
    Shader(const char *vertexPath, const char *fragmentPath);

    // Utiliser/activer le shader
    void use() const;

    // Fonctions utilitaires pour définir les uniforms
    void setBool(const std::string &name, bool value) const;
    void setInt(const std::string &name, int value) const;
    void setFloat(const std::string &name, float value) const;
    void setMat4(const std::string &name, const glm::mat4 &mat) const;
    void setVec3(const std::string &name, const glm::vec3 &vec) const;
    
    // Fonction pour lier les UBOs au shader
    void bindUBOs() const;
};

#endif
