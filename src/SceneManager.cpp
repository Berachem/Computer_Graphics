#include "SceneManager.h"
#include <iostream>

SceneManager::SceneManager() 
    : currentSceneIndex(-1), initialized(false) {
}

SceneManager::~SceneManager() {
    Cleanup();
}

bool SceneManager::Initialize(Camera& camera, SoundManager& soundManager) {
    if (initialized) return true;

    for (auto& scene : scenes) {
        if (!scene->Initialize(camera, soundManager)) {
            std::cerr << "Erreur : échec de l'initialisation de la scène " << scene->GetName() << std::endl;
            return false;
        }
    }

    if (!scenes.empty()) {
        currentSceneIndex = 0;
        scenes[currentSceneIndex]->OnActivate();
        std::cout << "Scène active : " << scenes[currentSceneIndex]->GetName() << std::endl;
    }

    initialized = true;
    return true;
}

void SceneManager::AddScene(std::unique_ptr<Scene> scene) {
    if (scene) {
        std::cout << "Ajout de la scène : " << scene->GetName() << std::endl;
        scenes.push_back(std::move(scene));
    }
}

void SceneManager::NextScene() {
    if (scenes.empty()) return;

    if (currentSceneIndex >= 0) {
        scenes[currentSceneIndex]->OnDeactivate();
    }

    currentSceneIndex = (currentSceneIndex + 1) % scenes.size();
    
    scenes[currentSceneIndex]->OnActivate();
    std::cout << "Changement vers la scène : " << scenes[currentSceneIndex]->GetName() << std::endl;
}

void SceneManager::PreviousScene() {
    if (scenes.empty()) return;

    if (currentSceneIndex >= 0) {
        scenes[currentSceneIndex]->OnDeactivate();
    }

    currentSceneIndex = (currentSceneIndex - 1 + scenes.size()) % scenes.size();
    
    scenes[currentSceneIndex]->OnActivate();
    std::cout << "Changement vers la scène : " << scenes[currentSceneIndex]->GetName() << std::endl;
}

bool SceneManager::SetCurrentScene(int index) {
    if (index < 0 || index >= static_cast<int>(scenes.size())) {
        std::cerr << "Erreur : index de scène invalide " << index << std::endl;
        return false;
    }

    if (index == currentSceneIndex) {
        return true; // Déjà sur cette scène
    }

    if (currentSceneIndex >= 0) {
        scenes[currentSceneIndex]->OnDeactivate();
    }

    currentSceneIndex = index;
    scenes[currentSceneIndex]->OnActivate();
    std::cout << "Changement vers la scène : " << scenes[currentSceneIndex]->GetName() << std::endl;
    return true;
}

void SceneManager::Update(float deltaTime, GLFWwindow* window, Camera& camera, SoundManager& soundManager) {
    if (!initialized || currentSceneIndex < 0 || currentSceneIndex >= static_cast<int>(scenes.size())) {
        return;
    }

    scenes[currentSceneIndex]->Update(deltaTime, window, camera, soundManager);
}

void SceneManager::Render(Camera& camera, int screenWidth, int screenHeight) {
    if (!initialized || currentSceneIndex < 0 || currentSceneIndex >= static_cast<int>(scenes.size())) {
        return;
    }

    scenes[currentSceneIndex]->Render(camera, screenWidth, screenHeight);
}

void SceneManager::RenderUI(GLFWwindow* window, SoundManager& soundManager) {
    if (!initialized || currentSceneIndex < 0 || currentSceneIndex >= static_cast<int>(scenes.size())) {
        return;
    }

    scenes[currentSceneIndex]->RenderUI(window, soundManager);
}

int SceneManager::GetSceneCount() const {
    return static_cast<int>(scenes.size());
}

int SceneManager::GetCurrentSceneIndex() const {
    return currentSceneIndex;
}

const char* SceneManager::GetCurrentSceneName() const {
    if (!initialized || currentSceneIndex < 0 || currentSceneIndex >= static_cast<int>(scenes.size())) {
        return "Aucune";
    }
    return scenes[currentSceneIndex]->GetName();
}

Scene* SceneManager::GetCurrentScene() const {
    if (!initialized || currentSceneIndex < 0 || currentSceneIndex >= static_cast<int>(scenes.size())) {
        return nullptr;
    }
    return scenes[currentSceneIndex].get();
}

bool SceneManager::IsInitialized() const {
    return initialized;
}

void SceneManager::Cleanup() {
    if (initialized) {
        if (currentSceneIndex >= 0 && currentSceneIndex < static_cast<int>(scenes.size())) {
            scenes[currentSceneIndex]->OnDeactivate();
        }

        for (auto& scene : scenes) {
            scene->Cleanup();
        }
    }

    scenes.clear();
    currentSceneIndex = -1;
    initialized = false;
    std::cout << "SceneManager nettoyé" << std::endl;
}
