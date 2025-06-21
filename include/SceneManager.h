#ifndef SCENEMANAGER_H
#define SCENEMANAGER_H

#include <memory>
#include <vector>
#include <string>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "Scene.h"
#include "Camera.h"
#include "SoundManager.h"

// Gestionnaire de scènes
class SceneManager {
private:
    std::vector<std::unique_ptr<Scene>> scenes;
    int currentSceneIndex;
    bool initialized;

public:
    SceneManager();
    ~SceneManager();
    bool Initialize(Camera& camera, SoundManager& soundManager);
    void AddScene(std::unique_ptr<Scene> scene);
    void NextScene();
    void PreviousScene();
    bool SetCurrentScene(int index);
    void Update(float deltaTime, GLFWwindow* window, Camera& camera, SoundManager& soundManager);
    void Render(Camera& camera, int screenWidth, int screenHeight);
    void RenderUI(GLFWwindow* window, SoundManager& soundManager);
    int GetSceneCount() const;
    int GetCurrentSceneIndex() const;
    const char* GetCurrentSceneName() const;
    Scene* GetCurrentScene() const;
    bool IsInitialized() const;
    void Cleanup();
};

#endif // SCENEMANAGER_H
