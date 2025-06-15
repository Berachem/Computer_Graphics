#include "LightScene.h"
#include "imgui.h"
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>

LightScene::LightScene()
    : lightPosition(-50.0f, 20.0f, -50.0f),  // Position statique éloignée
      lightColor(1.0f, 1.0f, 0.8f),
      lightRadius(5.0f),                      // Plus grande pour être visible de loin
      rotationSpeed(0.0f),                    // Pas de rotation
      currentRotation(0.0f),
      initialized(false) {
}

LightScene::~LightScene() {
    Cleanup();
}

bool LightScene::Initialize(Camera& camera, SoundManager& soundManager) {
    if (initialized) {
        return true;
    }

    std::cout << "Initialisation de la LightScene..." << std::endl;

    // Charger les shaders
    if (!LoadShaders()) {
        std::cerr << "Erreur : échec du chargement des shaders pour LightScene" << std::endl;
        return false;
    }

    // Créer la sphère de lumière
    if (!CreateLightSphere()) {
        std::cerr << "Erreur : échec de la création de la sphère de lumière" << std::endl;
        return false;
    }

    initialized = true;
    std::cout << "LightScene initialisée avec succès" << std::endl;
    return true;
}

bool LightScene::LoadShaders() {
    try {
        // Utiliser le shader du soleil pour la source de lumière
        lightShader = std::make_unique<Shader>("../shaders/sun.vert", "../shaders/sun.frag");
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Erreur lors du chargement des shaders LightScene : " << e.what() << std::endl;
        return false;
    }
}

bool LightScene::CreateLightSphere() {
    try {
        // Créer une sphère simple sans texture pour représenter la lumière
        lightSphere = std::make_unique<Sphere>("", lightRadius, 32, 16);
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Erreur lors de la création de la sphère de lumière : " << e.what() << std::endl;
        return false;
    }
}

void LightScene::Update(float deltaTime, GLFWwindow* window, Camera& camera, SoundManager& soundManager) {
    if (!initialized) return;

    // Lumière statique - pas de mise à jour de position nécessaire
    // La position reste fixe pour permettre l'ajout d'objets plus tard
}



void LightScene::Render(Camera& camera, int screenWidth, int screenHeight) {
    if (!initialized) return;

    RenderLight(camera, screenWidth, screenHeight);
}

void LightScene::RenderLight(Camera& camera, int screenWidth, int screenHeight) {
    // Matrices de projection et de vue
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), 
                                          (float)screenWidth / (float)screenHeight, 
                                          0.1f, 100.0f);
    glm::mat4 view = camera.GetViewMatrix();

    // Rendu de la sphère de lumière
    lightShader->use();
    lightShader->setMat4("projection", projection);
    lightShader->setMat4("view", view);
    lightShader->setFloat("time", static_cast<float>(glfwGetTime()));

    // Positionner la sphère de lumière
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, lightPosition);
    lightShader->setMat4("model", model);

    // Dessiner la sphère
    lightSphere->Draw(*lightShader);
}

void LightScene::RenderUI(GLFWwindow* window, SoundManager& soundManager) {
    if (!initialized) return;

    // Utiliser la même interface que MainScene - pas d'interface spécifique à la lumière
    // L'interface principale (contrôles principaux) est gérée par main.cpp
}



const char* LightScene::GetName() const {
    return "Scène de Lumière";
}

void LightScene::OnActivate() {
    std::cout << "LightScene activée - Scène d'éclairage statique prête pour l'ajout d'objets" << std::endl;
}

void LightScene::OnDeactivate() {
    std::cout << "LightScene désactivée" << std::endl;
}



void LightScene::Cleanup() {
    lightShader.reset();
    lightSphere.reset();

    initialized = false;
    std::cout << "LightScene nettoyée" << std::endl;
}
