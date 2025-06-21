#include "LightScene.h"
#include "UIHelpers.h"
#include "AudioSource.h"
#include "Sound.h"
#include "ShaderManager.h"
#include "UBO.h"
#include "imgui.h"
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include "Skybox.h"

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

    // Charger l'audio
    if (!LoadAudio(soundManager)) {
        std::cerr << "Avertissement : échec du chargement de l'audio pour LightScene" << std::endl;
        // Continuer sans audio
    }

    // Charger la skybox spatiale pour LightScene (différente de MainScene)
    currentSkyboxType = SkyboxManager::SkyboxType::SPACE;
    std::vector<std::string> skyboxFaces = SkyboxManager::GetSkyboxFaces(currentSkyboxType);
    skybox = std::make_unique<Skybox>(skyboxFaces);
    std::cout << "Skybox chargée pour LightScene: " << SkyboxManager::GetSkyboxName(currentSkyboxType) << std::endl;

    initialized = true;
    std::cout << "LightScene initialisée avec succès" << std::endl;
    return true;
}

bool LightScene::LoadShaders() {
    // Les shaders sont maintenant gérés par le ShaderManager global
    std::cout << "LightScene: Utilisation du ShaderManager global" << std::endl;
    return true;
}

bool LightScene::CreateLightSphere() {
    try {
        // Créer une sphère simple sans texture pour représenter la lumière
        lightSphere = std::make_unique<Sphere>("", lightRadius, 32, 16);
        
        // Ajouter une sphère de test pour comparer les shaders d'éclairage
        testSphere = std::make_unique<Sphere>("", 3.0f, 32, 16);
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Erreur lors de la création des sphères : " << e.what() << std::endl;
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
    // Rendu skybox
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)screenWidth / screenHeight, 0.1f, 100.0f);
    glm::mat4 view = camera.GetViewMatrix();
    if (skybox) skybox->Render(view, projection);
    // Rendu de la lumière
    RenderLight(camera, screenWidth, screenHeight);
}

void LightScene::RenderLight(Camera& camera, int screenWidth, int screenHeight) {
    // Obtenir le shader du soleil depuis le gestionnaire global
    Shader* sunShader = ShaderManager::getInstance().GetSunShader();
    
    if (!sunShader || !g_uboManager) return;

    // Rendu de la sphère de lumière
    sunShader->use();
    sunShader->setFloat("time", static_cast<float>(glfwGetTime()));

    // Positionner la sphère de lumière
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, lightPosition);
    g_uboManager->UpdateTransformUBO(model);

    // Dessiner la sphère
    lightSphere->Draw(*sunShader);

    // Rendu de la sphère de test pour comparer les shaders d'éclairage
    Shader* currentLightingShader = ShaderManager::getInstance().GetCurrentLightingShader();
    if (currentLightingShader && testSphere) {
        currentLightingShader->use();
        currentLightingShader->setVec3("objectColor", glm::vec3(0.8f, 0.3f, 0.1f)); // Couleur orange

        // Positionner la sphère de test à côté de la source de lumière
        glm::mat4 testModel = glm::mat4(1.0f);
        testModel = glm::translate(testModel, lightPosition + glm::vec3(10.0f, 0.0f, 0.0f));
        g_uboManager->UpdateTransformUBO(testModel);
        
        testSphere->Draw(*currentLightingShader);
    }
}

void LightScene::RenderUI(GLFWwindow* window, SoundManager& soundManager) {
    if (!initialized) return;

    // Interface audio mutualisée - passer nullptr pour que l'interface utilise le son actuel du SoundManager
    UIHelpers::RenderAudioUI(window, soundManager, ambientSource, nullptr);
    
    // Interface de contrôles principaux unifiée (shader + skybox)
    UIHelpers::RenderMainControlsUI(currentSkyboxType,
        [this](SkyboxManager::SkyboxType type) { ChangeSkybox(type); }
    );
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
    // Les shaders sont maintenant gérés par le ShaderManager global
    lightSphere.reset();
    testSphere.reset();

    initialized = false;
    std::cout << "LightScene nettoyée" << std::endl;
}

bool LightScene::LoadAudio(SoundManager& soundManager) {
    if (!soundManager.IsInitialized()) return false;
    soundManager.SetupAmbientAudio();
    zooSound = soundManager.GetAmbientSound();
    ambientSource = soundManager.GetAmbientSource();
    return (zooSound && ambientSource);
}

void LightScene::ChangeSkybox(SkyboxManager::SkyboxType newType) {
    if (newType != currentSkyboxType) {
        currentSkyboxType = newType;
        std::vector<std::string> skyboxFaces = SkyboxManager::GetSkyboxFaces(currentSkyboxType);
        skybox = std::make_unique<Skybox>(skyboxFaces);
        std::cout << "Skybox changée pour LightScene: " << SkyboxManager::GetSkyboxName(currentSkyboxType) << std::endl;
    }
}
