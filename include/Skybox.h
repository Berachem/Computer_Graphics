#ifndef SKYBOX_H
#define SKYBOX_H

#include <vector>
#include <string>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <memory>
#include "Shader.h"

class Skybox {
public:
    // Constructeur : charge les faces et initialise le VAO/VBO
    Skybox(const std::vector<std::string>& faces);
    ~Skybox();

    // Render : view should have translation removed, projection usual
    void Render(const glm::mat4& view, const glm::mat4& projection);

private:
    unsigned int skyboxVAO, skyboxVBO;
    unsigned int cubemapTexture;
    std::unique_ptr<Shader> shader;

    // Charge une cubemap à partir des 6 faces
    unsigned int LoadCubemap(const std::vector<std::string>& faces);
};

#endif // SKYBOX_H
