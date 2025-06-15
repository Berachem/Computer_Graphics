#include "SceneManager.h"
#include <iostream>

SceneManager::SceneManager() 
    : currentSceneIndex(-1), initialized(false) {
}

SceneManager::~SceneManager() {
    Cleanup();
}

bool SceneManager::Initialize(Camera& camera, SoundManager& soundManager) {
    if (initialized) {
        std::cout << "SceneManager déjà initialisé" << std::endl;
        return true;
    }

    // Initialiser toutes les scènes ajoutées
    for (auto& scene : scenes) {
        if (!scene->Initialize(camera, soundManager)) {
            std::cerr << "Erreur : échec de l'initialisation de la scène " << scene->GetName() << std::endl;
            return false;
        }
    }

    // Si on a des scènes, activer la première
    if (!scenes.empty()) {
        currentSceneIndex = 0;
        scenes[currentSceneIndex]->OnActivate();
        std::cout << "Scène active : " << scenes[currentSceneIndex]->GetName() << std::endl;
    }

    initialized = true;
    std::cout << "SceneManager initialisé avec " << scenes.size() << " scène(s)" << std::endl;
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

    // Désactiver la scène actuelle
    if (currentSceneIndex >= 0) {
        scenes[currentSceneIndex]->OnDeactivate();
    }

    // Passer à la scène suivante (avec bouclage)
    currentSceneIndex = (currentSceneIndex + 1) % scenes.size();
    
    // Activer la nouvelle scène
    scenes[currentSceneIndex]->OnActivate();
    std::cout << "Changement vers la scène : " << scenes[currentSceneIndex]->GetName() << std::endl;
}

void SceneManager::PreviousScene() {
    if (scenes.empty()) return;

    // Désactiver la scène actuelle
    if (currentSceneIndex >= 0) {
        scenes[currentSceneIndex]->OnDeactivate();
    }

    // Passer à la scène précédente (avec bouclage)
    currentSceneIndex = (currentSceneIndex - 1 + scenes.size()) % scenes.size();
    
    // Activer la nouvelle scène
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

    // Désactiver la scène actuelle
    if (currentSceneIndex >= 0) {
        scenes[currentSceneIndex]->OnDeactivate();
    }

    // Changer vers la nouvelle scène
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

bool SceneManager::IsInitialized() const {
    return initialized;
}

void SceneManager::Cleanup() {
    if (initialized) {
        // Désactiver la scène actuelle
        if (currentSceneIndex >= 0 && currentSceneIndex < static_cast<int>(scenes.size())) {
            scenes[currentSceneIndex]->OnDeactivate();
        }

        // Nettoyer toutes les scènes
        for (auto& scene : scenes) {
            scene->Cleanup();
        }
    }

    scenes.clear();
    currentSceneIndex = -1;
    initialized = false;
    std::cout << "SceneManager nettoyé" << std::endl;
}
