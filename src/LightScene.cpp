#include "LightScene.h"
#include "UIHelpers.h"
#include "AudioSource.h"
#include "Sound.h"
#include "ShaderManager.h"
#include "UBO.h"
#include "imgui.h"
#include <iostream>
#include <cstdlib>
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>
#include "Skybox.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

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
    }    // Créer la sphère de lumière
    if (!CreateLightSphere()) {
        std::cerr << "Erreur : échec de la création de la sphère de lumière" << std::endl;
        return false;
    }

    // Charger les modèles 3D spectaculaires
    if (!LoadModels()) {
        std::cerr << "Erreur : échec du chargement des modèles pour LightScene" << std::endl;
        return false;
    }// Charger l'audio
    if (!LoadAudio(soundManager)) {
        std::cerr << "Avertissement : échec du chargement de l'audio pour LightScene" << std::endl;
        // Continuer sans audio
    }
      // Charger la skybox spatiale pour LightScene (vue spectaculaire de l'espace)
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
        // Créer le soleil central imposant
        sunSphere = std::make_unique<Sphere>("", sunRadius, 64, 32);
        
        // Créer la lune avec texture
        moonSphere = std::make_unique<Sphere>("../textures/spherical_moon_texture.jpg", moonRadius, 36, 18);
        
        // Garder les anciennes sphères pour compatibilité
        lightSphere = std::make_unique<Sphere>("", lightRadius, 32, 16);
        testSphere = std::make_unique<Sphere>("", 3.0f, 32, 16);
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Erreur lors de la création des sphères : " << e.what() << std::endl;
        return false;
    }
}

void LightScene::Update(float deltaTime, GLFWwindow* window, Camera& camera, SoundManager& soundManager) {
    if (!initialized) return;

    // Animation de la lune en orbite lointaine
    moonCurrentAngle += moonOrbitSpeed * deltaTime;
    if (moonCurrentAngle > 2.0f * M_PI) moonCurrentAngle -= 2.0f * M_PI;
    
    // Animation de l'anneau d'astéroïdes
    for (int i = 0; i < ASTEROID_COUNT; ++i) {
        AsteroidData& asteroid = asteroids[i];
        asteroid.currentAngle += asteroid.orbitSpeed * deltaTime;
        if (asteroid.currentAngle > 2.0f * M_PI) asteroid.currentAngle -= 2.0f * M_PI;
    }
    
    // Animation de la flotte de vaisseaux spatiaux
    for (int i = 0; i < SPACESHIP_COUNT; ++i) {
        SpaceshipData& ship = spaceships[i];
        
        // Mise à jour de l'angle orbital
        ship.currentAngle += ship.orbitSpeed * deltaTime;
        if (ship.currentAngle > 2.0f * M_PI) {
            ship.currentAngle -= 2.0f * M_PI;
        }
        
        // Mise à jour des phases d'oscillation individuelles
        ship.randomPhase += deltaTime * 2.0f;
        if (ship.randomPhase > 2.0f * M_PI) {
            ship.randomPhase -= 2.0f * M_PI;
        }
        
        // Oscillations verticales et horizontales
        ship.heightPhase1 += deltaTime * ship.heightFreq1;
        ship.heightPhase2 += deltaTime * ship.heightFreq2;
        ship.heightPhase3 += deltaTime * ship.heightFreq3;
        ship.horizontalPhase1 += deltaTime * ship.horizontalFreq1;
        ship.horizontalPhase2 += deltaTime * ship.horizontalFreq2;
    }
    
    globalTime += deltaTime;
    
    // Mettre à jour tous les nouveaux éléments dynamiques
    UpdateStations(deltaTime);
    UpdateComets(deltaTime);
    UpdateDebris(deltaTime);
    UpdatePortals(deltaTime);
    UpdateSatellites(deltaTime);
    UpdateParticleClouds(deltaTime);
    UpdateInteractions(deltaTime);
}



void LightScene::Render(Camera& camera, int screenWidth, int screenHeight) {
    if (!initialized) return;
    
    // Rendu skybox
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)screenWidth / screenHeight, 0.1f, 1000.0f);
    glm::mat4 view = glm::mat4(glm::mat3(camera.GetViewMatrix())); // Remove translation for skybox
    if (skybox) skybox->Render(view, projection);
    
    // Rendu du soleil central imposant
    RenderSun(camera, screenWidth, screenHeight);
    
    // Rendu de l'anneau d'astéroïdes dense
    RenderAsteroidRing(camera, screenWidth, screenHeight);
    
    // Rendu de la flotte de vaisseaux spatiaux
    RenderSpaceships(camera, screenWidth, screenHeight);
    
    // Rendu de la lune en orbite lointaine
    RenderMoon(camera, screenWidth, screenHeight);
    
    // Garder le rendu de la lumière pour compatibilité
    RenderLight(camera, screenWidth, screenHeight);
    
    // Rendu de tous les nouveaux éléments dynamiques
    RenderStations(camera, screenWidth, screenHeight);
    RenderComets(camera, screenWidth, screenHeight);
    RenderDebris(camera, screenWidth, screenHeight);
    RenderPortals(camera, screenWidth, screenHeight);
    RenderSatellites(camera, screenWidth, screenHeight);
    RenderParticleClouds(camera, screenWidth, screenHeight);
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

    // Interface de contrôles clavier unifiée
    UIHelpers::RenderKeyboardUI(window);    // Interface audio avec le son spécifique de cette scène
    UIHelpers::RenderAudioUI(window, soundManager, ambientSource, zooSound, currentSoundName,
        [this, &soundManager](const std::string& soundName) -> bool {
            return this->ChangeSceneSound(soundName, soundManager);
        });
    
    // Interface de contrôles principaux unifiée (shader + skybox)
    UIHelpers::RenderMainControlsUI(currentSkyboxType,
        [this](SkyboxManager::SkyboxType type) { ChangeSkybox(type); }
    );
}

const char* LightScene::GetName() const {
    return "Scène de Lumière";
}

void LightScene::OnActivate() {
    std::cout << "LightScene activée - Son Zoo" << std::endl;
    // Toujours démarrer l'audio Zoo lors de l'activation
    if (ambientSource && zooSound) {
        ambientSource->Stop(); // S'assurer qu'il n'y a pas de son résiduel
        ambientSource->Play(zooSound, true);
        std::cout << "Démarrage de la musique Zoo" << std::endl;
    } else {
        std::cout << "LightScene: Impossible de démarrer la musique (source ou son manquant)" << std::endl;
    }
}

void LightScene::OnDeactivate() {
    std::cout << "LightScene désactivée" << std::endl;
    // Arrêter complètement l'audio pour cette scène (au lieu de pause)
    if (ambientSource) {
        std::cout << "LightScene: Source audio existe" << std::endl;
        if (ambientSource->IsPlaying() || ambientSource->IsPaused()) {
            std::cout << "LightScene: Arrêt de la musique..." << std::endl;
            ambientSource->Stop();
            std::cout << "Arrêt de la musique Zoo" << std::endl;
        } else {
            std::cout << "LightScene: Musique n'était pas en cours de lecture" << std::endl;
        }
    } else {
        std::cout << "LightScene: Pas de source audio disponible" << std::endl;
    }
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
    
    // Charger le son Zoo pour LightScene (complètement indépendant)
    std::string soundName = "Zoo";
    zooSound = soundManager.GetSound(soundName);
    if (!zooSound) {
        // Fallback si Zoo n'existe pas
        soundName = "spatial_theme";
        zooSound = soundManager.GetSound(soundName);
        std::cout << "LightScene: Fallback vers spatial_theme" << std::endl;
    } else {
        std::cout << "LightScene: Chargement de Zoo" << std::endl;
    }
    
    // Sauvegarder le nom du son actuel
    currentSoundName = soundName;
    
    // Créer une source audio INDEPENDANTE pour cette scène
    ambientSource = soundManager.CreateAudioSource();
    if (ambientSource) {
        ambientSource->SetPosition({0.0f, 0.0f, 0.0f});
        ambientSource->SetVolume(0.3f);
        std::cout << "LightScene: Source audio créée" << std::endl;
    }
    
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

bool LightScene::ChangeSceneSound(const std::string& soundName, SoundManager& soundManager) {
    auto newSound = soundManager.GetSound(soundName);
    if (!newSound) {
        std::cerr << "Son non trouvé: " << soundName << std::endl;
        return false;
    }
    
    // Sauvegarder l'état de lecture actuel
    bool wasPlaying = ambientSource && ambientSource->IsPlaying();
    bool wasPaused = ambientSource && ambientSource->IsPaused();
    
    // Arrêter le son actuel
    if (ambientSource && (wasPlaying || wasPaused)) {
        ambientSource->Stop();
    }
    
    // Changer le son et sauvegarder le nom
    zooSound = newSound;
    currentSoundName = soundName;
    
    // Reprendre la lecture si elle était en cours
    if (ambientSource && wasPlaying) {
        ambientSource->Play(zooSound, true);
        std::cout << "LightScene: Son changé vers " << soundName << " (reprise automatique)" << std::endl;
    } else {
        std::cout << "LightScene: Son changé vers " << soundName << std::endl;
    }
    
    return true;
}

std::string LightScene::GetCurrentSoundName() const {
    return currentSoundName;
}

// === NOUVELLES MÉTHODES SPECTACULAIRES ===

bool LightScene::LoadModels() {
    try {
        // Charger le modèle d'astéroïde (réutilisé pour tout l'anneau)
        asteroidModel = std::make_unique<Model>("../models/astroid.obj");
        
        // Charger le modèle de vaisseau spatial
        spaceshipModel = std::make_unique<Model>("../models/map-bump.obj");
        
        // Initialiser l'anneau d'astéroïdes avec des propriétés variées
        InitializeAsteroidRing();
        
        // Initialiser la flotte de vaisseaux spatiaux
        InitializeSpaceships();
        
        // Initialiser tous les nouveaux éléments dynamiques
        InitializeStations();
        InitializeComets();
        InitializeDebris();
        InitializePortals();
        InitializeSatellites();
        InitializeParticleClouds();
        
        // Initialiser les interactions
        globalTime = 0.0f;
        attractionMode = false;
        repulsionMode = false;
        attractionPoint = glm::vec3(0.0f);
        attractionStrength = 100.0f;
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Erreur lors du chargement des modèles pour LightScene : " << e.what() << std::endl;
        return false;
    }
}

void LightScene::InitializeAsteroidRing() {
    std::cout << "Initialisation de l'anneau d'astéroïdes dense..." << std::endl;
    
    for (int i = 0; i < ASTEROID_COUNT; ++i) {
        AsteroidData& asteroid = asteroids[i];
        
        // Répartition angulaire uniforme avec variations
        asteroid.angleOffset = (2.0f * M_PI * i / ASTEROID_COUNT) + 
                              ((rand() % 100) / 100.0f - 0.5f) * 0.5f;
        
        // Rayon orbital avec variations (anneau dense entre 150 et 250)
        float baseRadius = 150.0f + (100.0f * i / ASTEROID_COUNT);
        asteroid.radiusOffset = baseRadius + ((rand() % 100) / 100.0f - 0.5f) * 30.0f;
        
        // Échelle variée pour créer de la diversité visuelle
        asteroid.scale = 8.0f + ((rand() % 100) / 100.0f) * 12.0f;
        
        // Vitesse de rotation propre aléatoire
        asteroid.rotationSpeed = 0.5f + ((rand() % 100) / 100.0f) * 2.0f;
        if (rand() % 2) asteroid.rotationSpeed *= -1; // Rotation dans les deux sens
        
        // Axe de rotation aléatoire
        asteroid.rotationAxis = glm::normalize(glm::vec3(
            (rand() % 100) / 100.0f - 0.5f,
            (rand() % 100) / 100.0f - 0.5f,
            (rand() % 100) / 100.0f - 0.5f
        ));
        
        // Couleur variée (tons rocheux/métalliques)
        float colorVariation = (rand() % 100) / 100.0f;
        asteroid.color = glm::mix(
            glm::vec3(0.4f, 0.3f, 0.2f), // Brun rocheux
            glm::vec3(0.6f, 0.5f, 0.4f), // Beige métallique
            colorVariation
        );
        
        // Vitesse orbitale légèrement variée
        asteroid.orbitSpeed = 0.3f + ((rand() % 100) / 100.0f) * 0.4f;
        
        // Angle initial
        asteroid.currentAngle = asteroid.angleOffset;
    }
}

void LightScene::InitializeSpaceships() {
    std::cout << "Initialisation de la flotte de vaisseaux spatiaux..." << std::endl;
    
    for (int i = 0; i < SPACESHIP_COUNT; ++i) {
        SpaceshipData& ship = spaceships[i];
        
        // Couleurs variées pour la flotte
        if (i % 5 == 0) ship.color = glm::vec3(0.2f, 0.4f, 1.0f); // Bleu
        else if (i % 5 == 1) ship.color = glm::vec3(1.0f, 0.8f, 0.2f); // Doré
        else if (i % 5 == 2) ship.color = glm::vec3(0.8f, 0.2f, 0.2f); // Rouge
        else if (i % 5 == 3) ship.color = glm::vec3(0.2f, 0.8f, 0.2f); // Vert
        else ship.color = glm::vec3(0.9f, 0.9f, 0.9f); // Blanc/Argent
        
        // Position orbitale avec répartition en plusieurs anneaux
        ship.angleOffset = (2.0f * M_PI * i / SPACESHIP_COUNT) + 
                          ((rand() % 100) / 100.0f - 0.5f) * 0.3f;
        
        // Rayons orbitaux variés (plusieurs anneaux de vaisseaux)
        if (i < SPACESHIP_COUNT / 3) {
            ship.orbitRadius = 300.0f + ((rand() % 100) / 100.0f) * 50.0f; // Anneau intérieur
        } else if (i < 2 * SPACESHIP_COUNT / 3) {
            ship.orbitRadius = 400.0f + ((rand() % 100) / 100.0f) * 50.0f; // Anneau moyen
        } else {
            ship.orbitRadius = 500.0f + ((rand() % 100) / 100.0f) * 50.0f; // Anneau extérieur
        }
        
        // Vitesse orbitale inversement proportionnelle au rayon
        ship.orbitSpeed = 200.0f / ship.orbitRadius;
        
        ship.currentAngle = ship.angleOffset;
        
        // Décalages aléatoires pour mouvements naturels
        ship.randomOffset = glm::vec3(
            ((rand() % 100) / 100.0f - 0.5f) * 10.0f,
            ((rand() % 100) / 100.0f - 0.5f) * 10.0f,
            ((rand() % 100) / 100.0f - 0.5f) * 10.0f
        );
        ship.randomPhase = (rand() % 100) / 100.0f * 2.0f * M_PI;
        
        // Paramètres d'oscillation verticale individuels
        ship.heightFreq1 = 0.5f + ((rand() % 100) / 100.0f) * 1.0f;
        ship.heightFreq2 = 1.0f + ((rand() % 100) / 100.0f) * 1.5f;
        ship.heightFreq3 = 0.3f + ((rand() % 100) / 100.0f) * 0.7f;
        ship.heightAmp1 = 5.0f + ((rand() % 100) / 100.0f) * 10.0f;
        ship.heightAmp2 = 3.0f + ((rand() % 100) / 100.0f) * 6.0f;
        ship.heightAmp3 = 2.0f + ((rand() % 100) / 100.0f) * 4.0f;
        ship.heightPhase1 = (rand() % 100) / 100.0f * 2.0f * M_PI;
        ship.heightPhase2 = (rand() % 100) / 100.0f * 2.0f * M_PI;
        ship.heightPhase3 = (rand() % 100) / 100.0f * 2.0f * M_PI;
        
        // Paramètres d'oscillation horizontale individuels
        ship.horizontalFreq1 = 0.8f + ((rand() % 100) / 100.0f) * 1.2f;
        ship.horizontalFreq2 = 1.2f + ((rand() % 100) / 100.0f) * 1.8f;
        ship.horizontalAmp1 = 8.0f + ((rand() % 100) / 100.0f) * 12.0f;
        ship.horizontalAmp2 = 5.0f + ((rand() % 100) / 100.0f) * 8.0f;
        ship.horizontalPhase1 = (rand() % 100) / 100.0f * 2.0f * M_PI;
        ship.horizontalPhase2 = (rand() % 100) / 100.0f * 2.0f * M_PI;
        
        // Échelle variée
        ship.scale = 3.0f + ((rand() % 100) / 100.0f) * 2.0f;
    }
}

void LightScene::RenderSun(Camera& camera, int screenWidth, int screenHeight) {
    if (!sunSphere) return;
    
    Shader* sunShader = ShaderManager::getInstance().GetSunShader();
    if (!sunShader) return;
    
    sunShader->use();
    sunShader->setFloat("time", static_cast<float>(glfwGetTime()));
    
    glm::mat4 model = glm::translate(glm::mat4(1.0f), sunPosition);
    model = glm::scale(model, glm::vec3(sunRadius));
    
    glm::mat4 view = camera.GetViewMatrix();
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), 
                                          (float)screenWidth / screenHeight, 0.1f, 1000.0f);
    
    sunShader->setMat4("model", model);
    sunShader->setMat4("view", view);
    sunShader->setMat4("projection", projection);
    
    sunSphere->Draw(*sunShader);
}

void LightScene::RenderAsteroidRing(Camera& camera, int screenWidth, int screenHeight) {
    if (!asteroidModel) return;
    
    Shader* shader = ShaderManager::getInstance().GetCurrentLightingShader();
    if (!shader) return;
    
    shader->use();
    
    glm::mat4 view = camera.GetViewMatrix();
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), 
                                          (float)screenWidth / screenHeight, 0.1f, 1000.0f);
    
    shader->setMat4("view", view);
    shader->setMat4("projection", projection);
    
    for (int i = 0; i < ASTEROID_COUNT; ++i) {
        const AsteroidData& asteroid = asteroids[i];
        
        // Position orbitale
        float x = asteroid.radiusOffset * cos(asteroid.currentAngle);
        float z = asteroid.radiusOffset * sin(asteroid.currentAngle);
        float y = sin(asteroid.currentAngle * 3.0f) * 10.0f; // Légère ondulation verticale
        
        glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(x, y, z));
        
        // Rotation propre
        model = glm::rotate(model, 
                           asteroid.rotationSpeed * static_cast<float>(glfwGetTime()), 
                           asteroid.rotationAxis);
        
        // Échelle
        model = glm::scale(model, glm::vec3(asteroid.scale));
        
        shader->setMat4("model", model);
        shader->setVec3("objectColor", asteroid.color);
        
        asteroidModel->Draw(*shader);
    }
}

void LightScene::RenderSpaceships(Camera& camera, int screenWidth, int screenHeight) {
    if (!spaceshipModel) return;
    
    Shader* shader = ShaderManager::getInstance().GetCurrentLightingShader();
    if (!shader) return;
    
    shader->use();
    
    glm::mat4 view = camera.GetViewMatrix();
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), 
                                          (float)screenWidth / screenHeight, 0.1f, 1000.0f);
    
    shader->setMat4("view", view);
    shader->setMat4("projection", projection);
    
    for (int i = 0; i < SPACESHIP_COUNT; ++i) {
        const SpaceshipData& ship = spaceships[i];
        
        // Position orbitale de base
        float baseX = ship.orbitRadius * cos(ship.currentAngle);
        float baseZ = ship.orbitRadius * sin(ship.currentAngle);
        
        // Oscillations verticales complexes
        float heightOffset = ship.heightAmp1 * sin(ship.heightPhase1) +
                           ship.heightAmp2 * sin(ship.heightPhase2) +
                           ship.heightAmp3 * sin(ship.heightPhase3);
        
        // Oscillations horizontales
        float horizontalOffset1 = ship.horizontalAmp1 * sin(ship.horizontalPhase1);
        float horizontalOffset2 = ship.horizontalAmp2 * sin(ship.horizontalPhase2);
        
        // Position finale avec tous les mouvements
        glm::vec3 finalPos = glm::vec3(
            baseX + horizontalOffset1 * cos(ship.currentAngle + M_PI/2) + ship.randomOffset.x,
            heightOffset + ship.randomOffset.y,
            baseZ + horizontalOffset1 * sin(ship.currentAngle + M_PI/2) + ship.randomOffset.z
        );
        
        glm::mat4 model = glm::translate(glm::mat4(1.0f), finalPos);
        
        // Orientation vers le centre avec rotation
        glm::vec3 direction = glm::normalize(-finalPos);
        float rotationAngle = atan2(direction.x, direction.z);
        model = glm::rotate(model, rotationAngle, glm::vec3(0.0f, 1.0f, 0.0f));
        
        // Échelle
        model = glm::scale(model, glm::vec3(ship.scale));
        
        shader->setMat4("model", model);
        shader->setVec3("objectColor", ship.color);
        
        spaceshipModel->Draw(*shader);
    }
}

void LightScene::RenderMoon(Camera& camera, int screenWidth, int screenHeight) {
    if (!moonSphere) return;
    
    Shader* shader = ShaderManager::getInstance().GetCurrentLightingShader();
    if (!shader) return;
    
    shader->use();
    
    // Position orbitale de la lune
    float x = moonOrbitRadius * cos(moonCurrentAngle);
    float z = moonOrbitRadius * sin(moonCurrentAngle);
    float y = sin(moonCurrentAngle * 0.5f) * 20.0f; // Légère inclinaison orbitale
    
    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(x, y, z));
    
    // Rotation propre de la lune
    model = glm::rotate(model, 
                       moonSelfRotSpeed * static_cast<float>(glfwGetTime()), 
                       glm::vec3(0.0f, 1.0f, 0.1f));
    
    // Échelle
    model = glm::scale(model, glm::vec3(moonRadius));
    
    glm::mat4 view = camera.GetViewMatrix();
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), 
                                          (float)screenWidth / screenHeight, 0.1f, 1000.0f);
    
    shader->setMat4("model", model);
    shader->setMat4("view", view);
    shader->setMat4("projection", projection);
    shader->setVec3("objectColor", glm::vec3(0.9f, 0.9f, 0.8f)); // Couleur lunaire
    
    moonSphere->Draw(*shader);
}

// === IMPLÉMENTATION DES NOUVEAUX ÉLÉMENTS DYNAMIQUES ===

void LightScene::InitializeStations() {
    std::cout << "Initialisation des stations spatiales rotatives..." << std::endl;
    
    for (int i = 0; i < STATION_COUNT; ++i) {
        SpaceStation& station = stations[i];
        
        // Position orbitale autour du soleil
        float angle = (2.0f * M_PI * i / STATION_COUNT);
        float radius = 350.0f + (rand() % 100);
        station.position = glm::vec3(radius * cos(angle), 
                                   ((rand() % 100) - 50) * 2.0f, 
                                   radius * sin(angle));
        
        // Rotation propre
        station.rotationSpeed = 0.5f + (rand() % 100) / 200.0f;
        station.currentRotation = (rand() % 360) * M_PI / 180.0f;
        station.rotationAxis = glm::normalize(glm::vec3(
            (rand() % 100) / 100.0f - 0.5f,
            1.0f,
            (rand() % 100) / 100.0f - 0.5f
        ));
        
        // Apparence
        station.scale = 5.0f + (rand() % 100) / 50.0f;
        station.color = glm::vec3(0.7f + (rand() % 30) / 100.0f, 
                                 0.7f + (rand() % 30) / 100.0f, 
                                 0.9f);
        
        // Orbite
        station.orbitAngle = angle;
        station.orbitRadius = radius;
        station.orbitSpeed = 0.1f + (rand() % 50) / 500.0f;
        
        // Défenses
        station.turretRotation = 0.0f;
        station.turretSpeed = 2.0f + (rand() % 100) / 50.0f;
    }
}

void LightScene::InitializeComets() {
    std::cout << "Initialisation des comètes avec traînées..." << std::endl;
    
    comets.clear();
    for (int i = 0; i < COMET_COUNT; ++i) {
        Comet comet;
        
        // Position de départ aléatoire lointaine
        float angle = (rand() % 360) * M_PI / 180.0f;
        float distance = 800.0f + (rand() % 400);
        comet.position = glm::vec3(distance * cos(angle), 
                                  ((rand() % 200) - 100) * 3.0f, 
                                  distance * sin(angle));
        
        // Vitesse dirigée vers le centre avec variation
        glm::vec3 direction = glm::normalize(-comet.position);
        direction += glm::vec3((rand() % 100) / 200.0f - 0.25f,
                              (rand() % 100) / 200.0f - 0.25f,
                              (rand() % 100) / 200.0f - 0.25f);
        comet.velocity = direction * (30.0f + (rand() % 40));
        
        // Traînée
        comet.trailLength = 20 + (rand() % 30);
        comet.trailPositions.clear();
        for (int j = 0; j < comet.trailLength; ++j) {
            comet.trailPositions.push_back(comet.position);
        }
        
        // Apparence
        comet.color = glm::vec3(0.8f + (rand() % 20) / 100.0f,
                               0.6f + (rand() % 40) / 100.0f,
                               0.2f + (rand() % 30) / 100.0f);
        comet.brightness = 0.5f + (rand() % 50) / 100.0f;
        comet.pulseSpeed = 2.0f + (rand() % 100) / 50.0f;
        comet.currentPhase = (rand() % 360) * M_PI / 180.0f;
        comet.size = 2.0f + (rand() % 100) / 50.0f;
        
        comets.push_back(comet);
    }
}

void LightScene::InitializeDebris() {
    std::cout << "Initialisation des débris spatiaux..." << std::endl;
    
    debris.clear();
    for (int i = 0; i < DEBRIS_COUNT; ++i) {
        SpaceDebris d;
        
        // Position aléatoire dans un volume sphérique
        float radius = 100.0f + (rand() % 600);
        float theta = (rand() % 360) * M_PI / 180.0f;
        float phi = (rand() % 180) * M_PI / 180.0f;
        
        d.position = glm::vec3(radius * sin(phi) * cos(theta),
                              radius * cos(phi),
                              radius * sin(phi) * sin(theta));
        
        // Vitesse chaotique
        d.velocity = glm::vec3((rand() % 100) / 50.0f - 1.0f,
                              (rand() % 100) / 50.0f - 1.0f,
                              (rand() % 100) / 50.0f - 1.0f) * 5.0f;
        
        // Rotation chaotique
        d.angularVelocity = glm::vec3((rand() % 100) / 25.0f - 2.0f,
                                     (rand() % 100) / 25.0f - 2.0f,
                                     (rand() % 100) / 25.0f - 2.0f);
        d.rotation = glm::vec3(0.0f);
        
        // Apparence
        d.scale = 0.5f + (rand() % 100) / 100.0f;
        d.color = glm::vec3(0.3f + (rand() % 40) / 100.0f,
                           0.3f + (rand() % 40) / 100.0f,
                           0.3f + (rand() % 40) / 100.0f);
        
        // Durée de vie
        d.maxLifetime = 60.0f + (rand() % 120);
        d.lifetime = d.maxLifetime;
        
        debris.push_back(d);
    }
}

void LightScene::InitializePortals() {
    std::cout << "Initialisation des portails énergétiques..." << std::endl;
    
    for (int i = 0; i < PORTAL_COUNT; ++i) {
        EnergyPortal& portal = portals[i];
        
        // Position stratégique
        float angle = (2.0f * M_PI * i / PORTAL_COUNT);
        float radius = 600.0f + (rand() % 200);
        portal.position = glm::vec3(radius * cos(angle),
                                   ((rand() % 100) - 50) * 4.0f,
                                   radius * sin(angle));
        
        // Rotation
        portal.rotationSpeed = 1.0f + (rand() % 100) / 50.0f;
        portal.currentRotation = (rand() % 360) * M_PI / 180.0f;
        
        // Pulsation énergétique
        portal.pulseIntensity = 0.5f + (rand() % 50) / 100.0f;
        portal.pulseSpeed = 2.0f + (rand() % 100) / 50.0f;
        
        // Couleurs énergétiques
        portal.color1 = glm::vec3(0.2f + (rand() % 60) / 100.0f,
                                 0.6f + (rand() % 40) / 100.0f,
                                 1.0f);
        portal.color2 = glm::vec3(1.0f,
                                 0.2f + (rand() % 60) / 100.0f,
                                 0.8f + (rand() % 20) / 100.0f);
        
        portal.size = 8.0f + (rand() % 100) / 25.0f;
        portal.energyFlow = 0.0f;
    }
}

void LightScene::InitializeSatellites() {
    std::cout << "Initialisation des satellites en formation..." << std::endl;
    
    for (int i = 0; i < SATELLITE_COUNT; ++i) {
        Satellite& sat = satellites[i];
        
        // Formation en grille orbitale
        int layer = i / 10;
        int pos = i % 10;
        
        sat.orbitRadius = 250.0f + layer * 50.0f;
        sat.orbitSpeed = 0.8f - layer * 0.1f;
        sat.currentAngle = (2.0f * M_PI * pos / 10) + layer * 0.3f;
        
        sat.basePosition = glm::vec3(0.0f, layer * 20.0f - 40.0f, 0.0f);
        
        // Antennes rotatives
        sat.antennaRotation = glm::vec3(0.0f);
        sat.antennaSpeed = 1.0f + (rand() % 100) / 100.0f;
        
        // État et apparence
        sat.isActive = (rand() % 10) > 2; // 80% actifs
        sat.color = sat.isActive ? 
                   glm::vec3(0.2f, 1.0f, 0.3f) :  // Vert si actif
                   glm::vec3(0.8f, 0.2f, 0.2f);   // Rouge si inactif
        
        sat.signalPulse = (rand() % 360) * M_PI / 180.0f;
    }
}

void LightScene::InitializeParticleClouds() {
    std::cout << "Initialisation des nuages de particules énergétiques..." << std::endl;
    
    particleClouds.clear();
    for (int i = 0; i < PARTICLE_CLOUD_COUNT; ++i) {
        ParticleCloud cloud;
        
        // Position aléatoire
        float angle = (rand() % 360) * M_PI / 180.0f;
        float radius = 400.0f + (rand() % 300);
        cloud.center = glm::vec3(radius * cos(angle),
                                ((rand() % 200) - 100) * 2.0f,
                                radius * sin(angle));
        
        cloud.radius = 30.0f + (rand() % 50);
        cloud.rotationSpeed = 0.5f + (rand() % 100) / 200.0f;
        cloud.currentRotation = 0.0f;
        
        // Couleur énergétique
        cloud.color = glm::vec3(0.5f + (rand() % 50) / 100.0f,
                               0.2f + (rand() % 60) / 100.0f,
                               0.8f + (rand() % 20) / 100.0f);
        
        cloud.intensity = 0.3f + (rand() % 70) / 100.0f;
        cloud.pulseSpeed = 1.0f + (rand() % 100) / 100.0f;
        
        // Particules individuelles
        int particleCount = 50 + (rand() % 100);
        cloud.particlePositions.clear();
        cloud.particleVelocities.clear();
        
        for (int j = 0; j < particleCount; ++j) {
            // Position aléatoire dans la sphère
            float r = cloud.radius * pow((rand() % 1000) / 1000.0f, 1.0f/3.0f);
            float theta = (rand() % 360) * M_PI / 180.0f;
            float phi = (rand() % 180) * M_PI / 180.0f;
            
            glm::vec3 particlePos = glm::vec3(r * sin(phi) * cos(theta),
                                             r * cos(phi),
                                             r * sin(phi) * sin(theta));
            cloud.particlePositions.push_back(particlePos);
            
            // Vitesse orbitale autour du centre
            glm::vec3 velocity = glm::cross(particlePos, glm::vec3(0.0f, 1.0f, 0.0f)) * 0.1f;
            cloud.particleVelocities.push_back(velocity);
        }
        
        particleClouds.push_back(cloud);
    }
}

// === MÉTHODES DE MISE À JOUR ===

void LightScene::UpdateStations(float deltaTime) {
    for (int i = 0; i < STATION_COUNT; ++i) {
        SpaceStation& station = stations[i];
        
        // Rotation propre
        station.currentRotation += station.rotationSpeed * deltaTime;
        
        // Orbite autour du soleil
        station.orbitAngle += station.orbitSpeed * deltaTime;
        if (station.orbitAngle > 2.0f * M_PI) station.orbitAngle -= 2.0f * M_PI;
        
        station.position.x = station.orbitRadius * cos(station.orbitAngle);
        station.position.z = station.orbitRadius * sin(station.orbitAngle);
        
        // Rotation des tourelles défensives
        station.turretRotation += station.turretSpeed * deltaTime;
    }
}

void LightScene::UpdateComets(float deltaTime) {
    for (auto& comet : comets) {
        // Mouvement
        comet.position += comet.velocity * deltaTime;
        
        // Mise à jour de la traînée
        comet.trailPositions.insert(comet.trailPositions.begin(), comet.position);
        if (comet.trailPositions.size() > comet.trailLength) {
            comet.trailPositions.pop_back();
        }
        
        // Pulsation lumineuse
        comet.currentPhase += comet.pulseSpeed * deltaTime;
        comet.brightness = 0.5f + 0.5f * sin(comet.currentPhase);
        
        // Reset si trop loin du centre
        if (glm::length(comet.position) > 1200.0f) {
            // Nouvelle position lointaine
            float angle = (rand() % 360) * M_PI / 180.0f;
            float distance = 800.0f + (rand() % 400);
            comet.position = glm::vec3(distance * cos(angle), 
                                      ((rand() % 200) - 100) * 3.0f, 
                                      distance * sin(angle));
            
            // Nouvelle direction vers le centre
            glm::vec3 direction = glm::normalize(-comet.position);
            direction += glm::vec3((rand() % 100) / 200.0f - 0.25f,
                                  (rand() % 100) / 200.0f - 0.25f,
                                  (rand() % 100) / 200.0f - 0.25f);
            comet.velocity = direction * (30.0f + (rand() % 40));
            
            // Reset traînée
            comet.trailPositions.clear();
            for (int j = 0; j < comet.trailLength; ++j) {
                comet.trailPositions.push_back(comet.position);
            }
        }
    }
}

void LightScene::UpdateDebris(float deltaTime) {
    for (auto& d : debris) {
        // Mouvement
        d.position += d.velocity * deltaTime;
        d.rotation += d.angularVelocity * deltaTime;
        
        // Durée de vie
        d.lifetime -= deltaTime;
        
        // Respawn si nécessaire
        if (d.lifetime <= 0.0f) {
            // Nouvelle position
            float radius = 100.0f + (rand() % 600);
            float theta = (rand() % 360) * M_PI / 180.0f;
            float phi = (rand() % 180) * M_PI / 180.0f;
            
            d.position = glm::vec3(radius * sin(phi) * cos(theta),
                                  radius * cos(phi),
                                  radius * sin(phi) * sin(theta));
            
            // Nouvelle vitesse
            d.velocity = glm::vec3((rand() % 100) / 50.0f - 1.0f,
                                  (rand() % 100) / 50.0f - 1.0f,
                                  (rand() % 100) / 50.0f - 1.0f) * 5.0f;
            
            d.lifetime = d.maxLifetime;
        }
    }
}

void LightScene::UpdatePortals(float deltaTime) {
    for (int i = 0; i < PORTAL_COUNT; ++i) {
        EnergyPortal& portal = portals[i];
        
        // Rotation continue
        portal.currentRotation += portal.rotationSpeed * deltaTime;
        
        // Pulsation énergétique
        portal.energyFlow += portal.pulseSpeed * deltaTime;
        portal.pulseIntensity = 0.5f + 0.5f * sin(portal.energyFlow);
    }
}

void LightScene::UpdateSatellites(float deltaTime) {
    for (int i = 0; i < SATELLITE_COUNT; ++i) {
        Satellite& sat = satellites[i];
        
        // Orbite
        sat.currentAngle += sat.orbitSpeed * deltaTime;
        if (sat.currentAngle > 2.0f * M_PI) sat.currentAngle -= 2.0f * M_PI;
        
        // Rotation des antennes
        sat.antennaRotation.y += sat.antennaSpeed * deltaTime;
        
        // Pulsation des signaux
        sat.signalPulse += deltaTime * 4.0f;
        
        // Changement d'état aléatoire
        if ((rand() % 10000) < 5) { // 0.05% de chance par frame
            sat.isActive = !sat.isActive;
            sat.color = sat.isActive ? 
                       glm::vec3(0.2f, 1.0f, 0.3f) : 
                       glm::vec3(0.8f, 0.2f, 0.2f);
        }
    }
}

void LightScene::UpdateParticleClouds(float deltaTime) {
    for (auto& cloud : particleClouds) {
        // Rotation du nuage
        cloud.currentRotation += cloud.rotationSpeed * deltaTime;
        
        // Pulsation d'intensité
        cloud.intensity = 0.3f + 0.4f * sin(globalTime * cloud.pulseSpeed);
        
        // Mouvement des particules individuelles
        for (size_t i = 0; i < cloud.particlePositions.size(); ++i) {
            cloud.particlePositions[i] += cloud.particleVelocities[i] * deltaTime;
            
            // Garder les particules dans le rayon du nuage
            if (glm::length(cloud.particlePositions[i]) > cloud.radius) {
                cloud.particlePositions[i] = glm::normalize(cloud.particlePositions[i]) * cloud.radius;
                cloud.particleVelocities[i] = glm::cross(cloud.particlePositions[i], glm::vec3(0.0f, 1.0f, 0.0f)) * 0.1f;
            }
        }
    }
}

void LightScene::UpdateInteractions(float deltaTime) {
    // Interactions avec la souris (si disponible)
    // Attraction/répulsion des débris vers un point
    
    if (attractionMode || repulsionMode) {
        float strength = attractionMode ? attractionStrength : -attractionStrength;
        
        for (auto& d : debris) {
            glm::vec3 direction = attractionPoint - d.position;
            float distance = glm::length(direction);
            if (distance > 0.1f) {
                direction = glm::normalize(direction);
                d.velocity += direction * strength * deltaTime / (distance * distance + 1.0f);
            }
        }
        
        // Effet sur les comètes aussi
        for (auto& comet : comets) {
            glm::vec3 direction = attractionPoint - comet.position;
            float distance = glm::length(direction);
            if (distance > 0.1f) {
                direction = glm::normalize(direction);
                comet.velocity += direction * strength * 0.5f * deltaTime / (distance * distance + 1.0f);
            }
        }
    }
}

// === MÉTHODES DE RENDU ===

void LightScene::RenderStations(Camera& camera, int screenWidth, int screenHeight) {
    if (!spaceshipModel) return; // Utilise le modèle de vaisseau pour les stations
    
    Shader* shader = ShaderManager::getInstance().GetCurrentLightingShader();
    if (!shader) return;
    
    shader->use();
    glm::mat4 view = camera.GetViewMatrix();
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), 
                                          (float)screenWidth / screenHeight, 0.1f, 1000.0f);
    shader->setMat4("view", view);
    shader->setMat4("projection", projection);
    
    for (int i = 0; i < STATION_COUNT; ++i) {
        const SpaceStation& station = stations[i];
        
        glm::mat4 model = glm::translate(glm::mat4(1.0f), station.position);
        model = glm::rotate(model, station.currentRotation, station.rotationAxis);
        model = glm::scale(model, glm::vec3(station.scale));
        
        shader->setMat4("model", model);
        shader->setVec3("objectColor", station.color);
        
        spaceshipModel->Draw(*shader);
    }
}

void LightScene::RenderComets(Camera& camera, int screenWidth, int screenHeight) {
    if (!lightSphere) return;
    
    Shader* shader = ShaderManager::getInstance().GetSunShader();
    if (!shader) return;
    
    shader->use();
    glm::mat4 view = camera.GetViewMatrix();
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), 
                                          (float)screenWidth / screenHeight, 0.1f, 1000.0f);
    shader->setMat4("view", view);
    shader->setMat4("projection", projection);
    
    for (const auto& comet : comets) {
        // Rendre la tête de la comète
        glm::mat4 model = glm::translate(glm::mat4(1.0f), comet.position);
        model = glm::scale(model, glm::vec3(comet.size * comet.brightness));
        
        shader->setMat4("model", model);
        shader->setFloat("time", globalTime);
        
        lightSphere->Draw(*shader);
        
        // Rendre la traînée (points plus petits)
        for (size_t i = 1; i < comet.trailPositions.size(); ++i) {
            float trailIntensity = 1.0f - (float)i / comet.trailPositions.size();
            model = glm::translate(glm::mat4(1.0f), comet.trailPositions[i]);
            model = glm::scale(model, glm::vec3(comet.size * trailIntensity * 0.3f));
            
            shader->setMat4("model", model);
            lightSphere->Draw(*shader);
        }
    }
}

void LightScene::RenderDebris(Camera& camera, int screenWidth, int screenHeight) {
    if (!asteroidModel) return;
    
    Shader* shader = ShaderManager::getInstance().GetCurrentLightingShader();
    if (!shader) return;
    
    shader->use();
    glm::mat4 view = camera.GetViewMatrix();
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), 
                                          (float)screenWidth / screenHeight, 0.1f, 1000.0f);
    shader->setMat4("view", view);
    shader->setMat4("projection", projection);
    
    for (const auto& d : debris) {
        glm::mat4 model = glm::translate(glm::mat4(1.0f), d.position);
        model = glm::rotate(model, d.rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, d.rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, d.rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::scale(model, glm::vec3(d.scale));
        
        shader->setMat4("model", model);
        
        // Couleur qui s'estompe avec la durée de vie
        float lifeFactor = d.lifetime / d.maxLifetime;
        glm::vec3 fadedColor = d.color * lifeFactor;
        shader->setVec3("objectColor", fadedColor);
        
        asteroidModel->Draw(*shader);
    }
}

void LightScene::RenderPortals(Camera& camera, int screenWidth, int screenHeight) {
    if (!lightSphere) return;
    
    Shader* shader = ShaderManager::getInstance().GetSunShader();
    if (!shader) return;
    
    shader->use();
    glm::mat4 view = camera.GetViewMatrix();
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), 
                                          (float)screenWidth / screenHeight, 0.1f, 1000.0f);
    shader->setMat4("view", view);
    shader->setMat4("projection", projection);
    
    for (int i = 0; i < PORTAL_COUNT; ++i) {
        const EnergyPortal& portal = portals[i];
        
        // Anneau extérieur
        glm::mat4 model = glm::translate(glm::mat4(1.0f), portal.position);
        model = glm::rotate(model, portal.currentRotation, glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(portal.size * (1.0f + portal.pulseIntensity * 0.3f)));
        
        shader->setMat4("model", model);
        shader->setFloat("time", globalTime);
        
        lightSphere->Draw(*shader);
        
        // Anneau intérieur (couleur différente)
        model = glm::translate(glm::mat4(1.0f), portal.position);
        model = glm::rotate(model, -portal.currentRotation * 1.5f, glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(portal.size * 0.6f * (1.0f + portal.pulseIntensity * 0.5f)));
        
        shader->setMat4("model", model);
        lightSphere->Draw(*shader);
    }
}

void LightScene::RenderSatellites(Camera& camera, int screenWidth, int screenHeight) {
    if (!spaceshipModel) return;
    
    Shader* shader = ShaderManager::getInstance().GetCurrentLightingShader();
    if (!shader) return;
    
    shader->use();
    glm::mat4 view = camera.GetViewMatrix();
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), 
                                          (float)screenWidth / screenHeight, 0.1f, 1000.0f);
    shader->setMat4("view", view);
    shader->setMat4("projection", projection);
    
    for (int i = 0; i < SATELLITE_COUNT; ++i) {
        const Satellite& sat = satellites[i];
        
        // Position orbitale
        glm::vec3 pos = sat.basePosition + glm::vec3(
            sat.orbitRadius * cos(sat.currentAngle),
            0.0f,
            sat.orbitRadius * sin(sat.currentAngle)
        );
        
        glm::mat4 model = glm::translate(glm::mat4(1.0f), pos);
        model = glm::rotate(model, sat.antennaRotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(1.5f));
        
        shader->setMat4("model", model);
        
        // Couleur avec pulsation si actif
        glm::vec3 color = sat.color;
        if (sat.isActive) {
            float pulse = 1.0f + 0.3f * sin(sat.signalPulse);
            color *= pulse;
        }
        shader->setVec3("objectColor", color);
        
        spaceshipModel->Draw(*shader);
    }
}

void LightScene::RenderParticleClouds(Camera& camera, int screenWidth, int screenHeight) {
    if (!lightSphere) return;
    
    Shader* shader = ShaderManager::getInstance().GetSunShader();
    if (!shader) return;
    
    shader->use();
    glm::mat4 view = camera.GetViewMatrix();
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), 
                                          (float)screenWidth / screenHeight, 0.1f, 1000.0f);
    shader->setMat4("view", view);
    shader->setMat4("projection", projection);
    shader->setFloat("time", globalTime);
    
    for (const auto& cloud : particleClouds) {
        // Matrice de rotation du nuage
        glm::mat4 cloudRotation = glm::rotate(glm::mat4(1.0f), cloud.currentRotation, glm::vec3(0.0f, 1.0f, 0.0f));
        
        // Rendre chaque particule
        for (const auto& particlePos : cloud.particlePositions) {
            glm::vec3 worldPos = cloud.center + glm::vec3(cloudRotation * glm::vec4(particlePos, 1.0f));
            
            glm::mat4 model = glm::translate(glm::mat4(1.0f), worldPos);
            model = glm::scale(model, glm::vec3(0.5f * cloud.intensity));
            
            shader->setMat4("model", model);
            lightSphere->Draw(*shader);
        }
    }
}
