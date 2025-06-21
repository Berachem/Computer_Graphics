#ifndef SCENE_H
#define SCENE_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include "Camera.h"
#include "SoundManager.h"

// Classe abstraite pour une scène
class Scene {
public:
    virtual ~Scene() = default;
    virtual bool Initialize(Camera& camera, SoundManager& soundManager) = 0;
    virtual void Update(float deltaTime, GLFWwindow* window, Camera& camera, SoundManager& soundManager) = 0;
    virtual void Render(Camera& camera, int screenWidth, int screenHeight) = 0;
    virtual void RenderUI(GLFWwindow* window, SoundManager& soundManager) = 0;
    virtual void Cleanup() = 0;
};

#endif
