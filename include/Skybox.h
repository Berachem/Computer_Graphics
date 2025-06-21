#ifndef SKYBOX_H
#define SKYBOX_H

#include <vector>
#include <string>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <memory>
#include "Shader.h"

// Skybox cubemap
class Skybox {
public:
    Skybox(const std::vector<std::string>& faces);
    ~Skybox();

    void Render(const glm::mat4& view, const glm::mat4& projection);

private:
    unsigned int skyboxVAO, skyboxVBO;
    unsigned int cubemapTexture;
    std::unique_ptr<Shader> shader;

    unsigned int LoadCubemap(const std::vector<std::string>& faces);
};

#endif // SKYBOX_H
