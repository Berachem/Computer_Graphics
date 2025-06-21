#include "MainScene.h"
#include "Skybox.h"
#include "UIHelpers.h"
#include "ShaderManager.h"
#include "UBO.h"
#include "imgui.h"
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>

// Variables pour les animations (extraites de main.cpp)
static float luneOrbitRadius = 10.0f;
static float luneOrbitSpeed = 0.25f;
static float luneSelfRotSpeed = 1.0f;
static float sunDistance = 100.0f;
static glm::vec3 lightPosition = glm::vec3(-100.0f, 15.0f, -100.0f);

MainScene::MainScene() 
    : sunRadius(45.0f), initialized(false) {
}

MainScene::~MainScene() {
    Cleanup();
}

bool MainScene::Initialize(Camera& camera, SoundManager& soundManager) {
    if (initialized) {
        return true;
    }

    std::cout << "Initialisation de la MainScene..." << std::endl;

    // Charger les shaders
    if (!LoadShaders()) {
        std::cerr << "Erreur : échec du chargement des shaders" << std::endl;
        return false;
    }

    // Charger les modèles
    if (!LoadModels()) {
        std::cerr << "Erreur : échec du chargement des modèles" << std::endl;
        return false;
    }

    // Charger l'audio
    if (!LoadAudio(soundManager)) {
        std::cerr << "Avertissement : échec du chargement de l'audio" << std::endl;
        // Continuer sans audio
    }

    // Charger la skybox colorée pour MainScene
    currentSkyboxType = SkyboxManager::SkyboxType::COLORER;
    std::vector<std::string> skyboxFaces = SkyboxManager::GetSkyboxFaces(currentSkyboxType);
    skybox = std::make_unique<Skybox>(skyboxFaces);
    std::cout << "Skybox chargée pour MainScene: " << SkyboxManager::GetSkyboxName(currentSkyboxType) << std::endl;

    initialized = true;
    std::cout << "MainScene initialisée avec succès" << std::endl;
    return true;
}

bool MainScene::LoadShaders() {
    // Les shaders sont maintenant gérés par le ShaderManager global
    // Pas besoin de les charger ici, ils sont déjà initialisés dans main.cpp
    std::cout << "MainScene: Utilisation du ShaderManager global" << std::endl;
    return true;
}

bool MainScene::LoadModels() {
    try {
        myModel = std::make_unique<Model>("../models/map-bump.obj");
        asteroid1 = std::make_unique<Model>("../models/astroid.obj");
        asteroid2 = std::make_unique<Model>("../models/astroid.obj");
        asteroid3 = std::make_unique<Model>("../models/astroid.obj");
        asteroid4 = std::make_unique<Model>("../models/astroid.obj");
        
        moonSphere = std::make_unique<Sphere>("../textures/spherical_moon_texture.jpg", 0.5f, 36, 18);
        sunSphere = std::make_unique<Sphere>("", sunRadius, 36, 18);
        
        // Ajouter une sphère de test simple pour comparer les shaders d'éclairage
        testSphere = std::make_unique<Sphere>("", 2.0f, 32, 16);
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Erreur lors du chargement des modèles : " << e.what() << std::endl;
        return false;
    }
}

bool MainScene::LoadAudio(SoundManager& soundManager) {
    if (!soundManager.IsInitialized()) return false;
    soundManager.SetupAmbientAudio();
    zooSound = soundManager.GetAmbientSound();
    ambientSource = soundManager.GetAmbientSource();
    return (zooSound && ambientSource);
}

void MainScene::Update(float deltaTime, GLFWwindow* window, Camera& camera, SoundManager& soundManager) {
    if (!initialized) return;

    // Mettre à jour la position de l'auditeur avec la caméra
    if (soundManager.IsInitialized()) {
        float listenerPos[3] = {camera.Position.x, camera.Position.y, camera.Position.z};
        float listenerForward[3] = {camera.Front.x, camera.Front.y, camera.Front.z};
        float listenerUp[3] = {camera.Up.x, camera.Up.y, camera.Up.z};
        soundManager.SetListenerPosition(listenerPos, listenerForward, listenerUp);
    }

    // Mettre à jour la position du soleil (rotation lente)
    float currentFrame = static_cast<float>(glfwGetTime());
    float angle = currentFrame * 0.0005f;
    lightPosition.x = -sunDistance * cos(angle);
    lightPosition.z = -sunDistance * sin(angle);
    lightPosition.y = 15.0f;
}

void MainScene::Render(Camera& camera, int screenWidth, int screenHeight) {
    if (!initialized) return;

    RenderObjects(camera, screenWidth, screenHeight);
}

void MainScene::RenderUI(GLFWwindow* window, SoundManager& soundManager) {
    if (!initialized) return;

    // Interface audio mutualisée - passer nullptr pour que l'interface utilise le son actuel du SoundManager
    UIHelpers::RenderAudioUI(window, soundManager, ambientSource, nullptr);
    
    // Interface de contrôles principaux unifiée (shader + skybox)
    UIHelpers::RenderMainControlsUI(currentSkyboxType,
        [this](SkyboxManager::SkyboxType type) { ChangeSkybox(type); }
    );
}

const char* MainScene::GetName() const {
    return "Scène Principale";
}

void MainScene::OnActivate() {
    std::cout << "MainScene activée" << std::endl;
    // Reprendre l'audio si nécessaire (une seule fois)
    if (ambientSource && zooSound && !ambientSource->IsPlaying()) {
        ambientSource->Play(zooSound, true);
    }
}

void MainScene::OnDeactivate() {
    std::cout << "MainScene désactivée" << std::endl;
    // Optionnel : mettre en pause l'audio
}

void MainScene::RenderObjects(Camera& camera, int screenWidth, int screenHeight) {
    float currentFrame = static_cast<float>(glfwGetTime());

    // Rendu de la skybox en premier
    if (skybox) {
        // La skybox utilise son propre shader qui n'a pas besoin d'UBO de transformation
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), 
                                               (float)screenWidth / screenHeight, 
                                               0.1f, 1000.0f);
        glm::mat4 view = camera.GetViewMatrix();
        skybox->Render(view, projection);
    }

    // Obtenir les shaders depuis le gestionnaire global
    Shader* metalShader = ShaderManager::getInstance().GetMetalShader();
    Shader* texturedShader = ShaderManager::getInstance().GetTexturedShader();
    Shader* sunShader = ShaderManager::getInstance().GetSunShader();
    Shader* currentLightingShader = ShaderManager::getInstance().GetCurrentLightingShader();    //======== VAISSEAU (utilise le shader d'éclairage sélectionné pour voir la différence) ========
    if (currentLightingShader && g_uboManager) {
        currentLightingShader->use();
        
        glm::mat4 model = glm::mat4(1.0f);
        g_uboManager->UpdateTransformUBO(model);
        
        // Couleur gris métallique pour bien voir les effets d'éclairage
        currentLightingShader->setVec3("objectColor", glm::vec3(0.7f, 0.7f, 0.8f));
        myModel->Draw(*currentLightingShader);
    }

    //======== LUNE (utilise le shader texturé avec UBO) ========
    if (texturedShader && g_uboManager) {
        texturedShader->use();

        // Orbite de la lune
        float orbitAngle = currentFrame * luneOrbitSpeed;
        float sphereX = luneOrbitRadius * cos(orbitAngle);
        float sphereZ = luneOrbitRadius * sin(orbitAngle);

        glm::mat4 sphereModel = glm::mat4(1.0f);
        sphereModel = glm::translate(sphereModel, glm::vec3(sphereX, 2.0f, sphereZ));
        sphereModel = glm::rotate(sphereModel, currentFrame * luneSelfRotSpeed, glm::vec3(0.0f, 1.0f, 0.0f));
        g_uboManager->UpdateTransformUBO(sphereModel);
        
        moonSphere->Draw(*texturedShader);
    }

    //======== SOLEIL (utilise son shader spécialisé) ========
    if (sunShader && g_uboManager) {
        sunShader->use();
        sunShader->setFloat("time", currentFrame);

        glm::mat4 sunModel = glm::mat4(1.0f);
        sunModel = glm::translate(sunModel, lightPosition);
        g_uboManager->UpdateTransformUBO(sunModel);
        
        sunSphere->Draw(*sunShader);
    }    //======== ASTEROIDS (utilisent le shader d'éclairage sélectionné) ========
    if (currentLightingShader && g_uboManager) {
        float asteroidOrbitAngle = currentFrame * 0.5f;
        float asteroidOrbitRadius = 60.0f;

        // Astéroïde 1 - Couleur rouge
        currentLightingShader->use();
        currentLightingShader->setVec3("objectColor", glm::vec3(0.8f, 0.3f, 0.2f)); // Rouge
        float inclinationAngle = glm::radians(15.0f);
        glm::mat4 inclinationMatrix = glm::rotate(glm::mat4(1.0f), inclinationAngle, glm::vec3(1.0f, 0.0f, 0.0f));
        glm::vec3 asteroidPosition = glm::vec3(
            asteroidOrbitRadius * cos(asteroidOrbitAngle),
            0.0f,
            asteroidOrbitRadius * sin(asteroidOrbitAngle)
        );
        glm::vec4 inclinedPosition = inclinationMatrix * glm::vec4(asteroidPosition, 1.0f);

        glm::mat4 asteroidModel = glm::mat4(1.0f);
        asteroidModel = glm::translate(asteroidModel, glm::vec3(lightPosition.x + inclinedPosition.x,
                                                               lightPosition.y + inclinedPosition.y,
                                                               lightPosition.z + inclinedPosition.z));
        asteroidModel = glm::scale(asteroidModel, glm::vec3(0.05f, 0.05f, 0.05f));
        asteroidModel = glm::rotate(asteroidModel, asteroidOrbitAngle * 10.0f, glm::vec3(0.0f, 1.0f, 0.5f));
        g_uboManager->UpdateTransformUBO(asteroidModel);
        asteroid1->Draw(*currentLightingShader);

        // Astéroïde 2 - Couleur bleue
        currentLightingShader->setVec3("objectColor", glm::vec3(0.2f, 0.4f, 0.8f)); // Bleu
        inclinationAngle = glm::radians(11.0f);
        inclinationMatrix = glm::rotate(glm::mat4(1.0f), inclinationAngle, glm::vec3(1.0f, 0.0f, 0.0f));
        glm::vec3 asteroid2Position = glm::vec3(
            asteroidOrbitRadius * cos(asteroidOrbitAngle + glm::radians(10.0f)),
            0.0f,
            asteroidOrbitRadius * sin(asteroidOrbitAngle + glm::radians(10.0f))
        );
        glm::vec4 inclinedPosition2 = inclinationMatrix * glm::vec4(asteroid2Position, 1.0f);

        glm::mat4 asteroid2Model = glm::mat4(1.0f);
        asteroid2Model = glm::translate(asteroid2Model, glm::vec3(lightPosition.x + inclinedPosition2.x,
                                                                 lightPosition.y + inclinedPosition2.y,
                                                                 lightPosition.z + inclinedPosition2.z));
        asteroid2Model = glm::scale(asteroid2Model, glm::vec3(0.025f, 0.042f, 0.015f));
        asteroid2Model = glm::rotate(asteroid2Model, asteroidOrbitAngle * 7.0f, glm::vec3(0.5f, 1.0f, 0.0f));
        g_uboManager->UpdateTransformUBO(asteroid2Model);        asteroid2->Draw(*currentLightingShader);

        // Astéroïde 3 - Couleur jaune
        currentLightingShader->setVec3("objectColor", glm::vec3(0.8f, 0.8f, 0.2f)); // Jaune
        
        // Astéroïde 3
        inclinationAngle = glm::radians(0.0f);
        inclinationMatrix = glm::rotate(glm::mat4(1.0f), inclinationAngle, glm::vec3(1.0f, 0.0f, 0.0f));
        glm::vec3 asteroid3Position = glm::vec3(
            asteroidOrbitRadius * cos(asteroidOrbitAngle + glm::radians(15.0f)),
            0.0f,
            asteroidOrbitRadius * sin(asteroidOrbitAngle + glm::radians(15.0f))
        );
        glm::vec4 inclinedPosition3 = inclinationMatrix * glm::vec4(asteroid3Position, 1.0f);

        glm::mat4 asteroid3Model = glm::mat4(1.0f);
        asteroid3Model = glm::translate(asteroid3Model, glm::vec3(lightPosition.x + inclinedPosition3.x,
                                                                 lightPosition.y + inclinedPosition3.y,
                                                                 lightPosition.z + inclinedPosition3.z));
        asteroid3Model = glm::scale(asteroid3Model, glm::vec3(0.08f, 0.08f, 0.08f));
        asteroid3Model = glm::rotate(asteroid3Model, asteroidOrbitAngle * 12.0f, glm::vec3(1.0f, 0.0f, 1.0f));
        g_uboManager->UpdateTransformUBO(asteroid3Model);
        asteroid3->Draw(*currentLightingShader);

        // Astéroïde 4 - Couleur violette
        currentLightingShader->setVec3("objectColor", glm::vec3(0.6f, 0.2f, 0.8f)); // Violet
        inclinationAngle = glm::radians(8.0f);
        inclinationMatrix = glm::rotate(glm::mat4(1.0f), inclinationAngle, glm::vec3(1.0f, 0.0f, 0.0f));
        glm::vec3 asteroid4Position = glm::vec3(
            asteroidOrbitRadius * cos(asteroidOrbitAngle + glm::radians(25.0f)),
            0.0f,
            asteroidOrbitRadius * sin(asteroidOrbitAngle + glm::radians(25.0f))
        );
        glm::vec4 inclinedPosition4 = inclinationMatrix * glm::vec4(asteroid4Position, 1.0f);

        glm::mat4 asteroid4Model = glm::mat4(1.0f);
        asteroid4Model = glm::translate(asteroid4Model, glm::vec3(lightPosition.x + inclinedPosition4.x,
                                                                 lightPosition.y + inclinedPosition4.y,
                                                                 lightPosition.z + inclinedPosition4.z));
        asteroid4Model = glm::scale(asteroid4Model, glm::vec3(0.012f, 0.014f, 0.013f));
        asteroid4Model = glm::rotate(asteroid4Model, asteroidOrbitAngle * 5.0f, glm::vec3(0.3f, 0.7f, 0.2f));
        g_uboManager->UpdateTransformUBO(asteroid4Model);
        asteroid4->Draw(*currentLightingShader);
    }

    //======== SPHÈRE DE TEST (pour comparer Phong vs Lambert) ========
    if (currentLightingShader && g_uboManager) {
        currentLightingShader->use();
        currentLightingShader->setVec3("objectColor", glm::vec3(0.2f, 0.8f, 0.2f)); // Couleur verte

        // Positionner la sphère de test à côté du vaisseau
        glm::mat4 testModel = glm::mat4(1.0f);
        testModel = glm::translate(testModel, glm::vec3(5.0f, 0.0f, 0.0f)); // À droite du vaisseau
        g_uboManager->UpdateTransformUBO(testModel);
        testSphere->Draw(*currentLightingShader);
    }
}

void MainScene::Cleanup() {
    if (ambientSource) {
        ambientSource->Stop();
    }    // Les modèles 3D se nettoient automatiquement avec les smart pointers
    myModel.reset();
    asteroid1.reset();
    asteroid2.reset();
    asteroid3.reset();
    asteroid4.reset();

    moonSphere.reset();
    sunSphere.reset();
    testSphere.reset();

    moonSphere.reset();
    sunSphere.reset();

    initialized = false;
    std::cout << "MainScene nettoyée" << std::endl;
}

void MainScene::ChangeSkybox(SkyboxManager::SkyboxType newType) {
    if (newType != currentSkyboxType) {
        currentSkyboxType = newType;
        std::vector<std::string> skyboxFaces = SkyboxManager::GetSkyboxFaces(currentSkyboxType);
        skybox = std::make_unique<Skybox>(skyboxFaces);
        std::cout << "Skybox changée pour MainScene: " << SkyboxManager::GetSkyboxName(currentSkyboxType) << std::endl;
    }
}
