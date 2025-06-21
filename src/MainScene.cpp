#include "MainScene.h"
#include "Skybox.h"
#include "UIHelpers.h"
#include "ShaderManager.h"
#include "UBO.h"
#include "imgui.h"
#include <iostream>
#include <cstdlib>
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Variables pour les animations (extraites de main.cpp)
static float luneOrbitRadius = 250.0f;    // Lune plus éloignée que les astéroïdes (qui sont à ~60)
static float luneOrbitSpeed = 0.50f;     // Vitesse plus lente pour une orbite plus lointaine
static float luneSelfRotSpeed = 0.5f;
static float sunDistance = 100.0f;
static glm::vec3 lightPosition = glm::vec3(-100.0f, 15.0f, -100.0f);

MainScene::MainScene() 
    : sunRadius(45.0f), initialized(false), pilotMode(false), currentSpaceshipIndex(0), lastSpaceshipPosition(0.0f) {
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
    }    // Charger la skybox spatiale pour MainScene (vue d'ensemble de l'espace)
    currentSkyboxType = SkyboxManager::SkyboxType::SPACE;
    std::vector<std::string> skyboxFaces = SkyboxManager::GetSkyboxFaces(currentSkyboxType);
    skybox = std::make_unique<Skybox>(skyboxFaces);
    std::cout << "Skybox chargée pour MainScene: " << SkyboxManager::GetSkyboxName(currentSkyboxType) << std::endl;

    // Positionner la caméra pour une vue d'ensemble
    SetupCameraOverview(camera);

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
        // Charger un seul modèle d'astéroïde (réutilisé pour tout l'anneau)
        asteroidModel = std::make_unique<Model>("../models/astroid.obj");
        
        // Charger le modèle de vaisseau (réutilisé pour les 3 vaisseaux français)
        spaceshipModel = std::make_unique<Model>("../models/map-bump.obj"); // Utilise le même modèle pour les vaisseaux
        
        // Initialiser l'anneau d'astéroïdes avec des propriétés variées
        InitializeAsteroidRing();
        
        // Initialiser les vaisseaux français
        InitializeSpaceships();
        
        moonSphere = std::make_unique<Sphere>("../textures/spherical_moon_texture.jpg", 9.0f, 36, 18); // Lune très imposante (x3)
        sunSphere = std::make_unique<Sphere>("", sunRadius, 36, 18);
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Erreur lors du chargement des modèles : " << e.what() << std::endl;
        return false;
    }
}

bool MainScene::LoadAudio(SoundManager& soundManager) {
    if (!soundManager.IsInitialized()) return false;
    
    // Charger le thème spatial pour MainScene (complètement indépendant)
    std::string soundName = "spatial_theme";
    zooSound = soundManager.GetSound(soundName);
    if (!zooSound) {
        // Fallback si spatial_theme n'existe pas
        soundName = "Zoo";
        zooSound = soundManager.GetSound(soundName);
        std::cout << "MainScene: Fallback vers Zoo" << std::endl;
    } else {
        std::cout << "MainScene: Chargement de spatial_theme" << std::endl;
    }
    
    // Sauvegarder le nom du son actuel
    currentSoundName = soundName;
    
    // Créer une source audio INDEPENDANTE pour cette scène
    ambientSource = soundManager.CreateAudioSource();
    if (ambientSource) {
        ambientSource->SetPosition({0.0f, 0.0f, 0.0f});
        ambientSource->SetVolume(0.3f);
        std::cout << "MainScene: Source audio créée" << std::endl;
    }
    
    return (zooSound && ambientSource);
}

void MainScene::Update(float deltaTime, GLFWwindow* window, Camera& camera, SoundManager& soundManager) {
    if (!initialized) return;

    // Gestion de la touche V pour basculer le mode pilote
    static bool vKeyPressed = false;
    bool vKeyDown = (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS);
    if (vKeyDown && !vKeyPressed) {
        TogglePilotMode();
        std::cout << "Mode pilote " << (pilotMode ? "activé" : "désactivé") << std::endl;
        if (pilotMode) {
            std::cout << "Suivi du vaisseau " << (currentSpaceshipIndex + 1) << " (couleur: " 
                      << (currentSpaceshipIndex == 0 ? "Bleu" : currentSpaceshipIndex == 1 ? "Blanc" : "Rouge") << ")" << std::endl;
        }
    }
    vKeyPressed = vKeyDown;

    // Mettre à jour la position de l'auditeur avec la caméra
    if (soundManager.IsInitialized()) {
        float listenerPos[3] = {camera.Position.x, camera.Position.y, camera.Position.z};
        float listenerForward[3] = {camera.Front.x, camera.Front.y, camera.Front.z};
        float listenerUp[3] = {camera.Up.x, camera.Up.y, camera.Up.z};
        soundManager.SetListenerPosition(listenerPos, listenerForward, listenerUp);
    }// Mettre à jour la position du soleil (rotation lente)
    float currentFrame = static_cast<float>(glfwGetTime());
    float angle = currentFrame * 0.0005f;
    lightPosition.x = -sunDistance * cos(angle);
    lightPosition.z = -sunDistance * sin(angle);
    lightPosition.y = 15.0f;
    
    // Mettre à jour les vaisseaux français
    for (int i = 0; i < SPACESHIP_COUNT; ++i) {
        SpaceshipData& ship = spaceships[i];
        
        // Mise à jour de l'angle orbital
        ship.currentAngle += ship.orbitSpeed * deltaTime;
        if (ship.currentAngle > 2.0f * M_PI) {
            ship.currentAngle -= 2.0f * M_PI;
        }
          // Mise à jour des phases d'oscillation individuelles pour des mouvements naturels
        ship.randomPhase += deltaTime * 2.0f; // Phase générale
        if (ship.randomPhase > 2.0f * M_PI) {
            ship.randomPhase -= 2.0f * M_PI;
        }
        
        // Mise à jour des phases verticales individuelles
        ship.heightPhase1 += deltaTime * ship.heightFreq1;
        ship.heightPhase2 += deltaTime * ship.heightFreq2;
        ship.heightPhase3 += deltaTime * ship.heightFreq3;
        
        // Mise à jour des phases horizontales individuelles
        ship.horizontalPhase1 += deltaTime * ship.horizontalFreq1;
        ship.horizontalPhase2 += deltaTime * ship.horizontalFreq2;
        
        // Normaliser les phases pour éviter les débordements
        if (ship.heightPhase1 > 2.0f * M_PI) ship.heightPhase1 -= 2.0f * M_PI;
        if (ship.heightPhase2 > 2.0f * M_PI) ship.heightPhase2 -= 2.0f * M_PI;
        if (ship.heightPhase3 > 2.0f * M_PI) ship.heightPhase3 -= 2.0f * M_PI;
        if (ship.horizontalPhase1 > 2.0f * M_PI) ship.horizontalPhase1 -= 2.0f * M_PI;
        if (ship.horizontalPhase2 > 2.0f * M_PI) ship.horizontalPhase2 -= 2.0f * M_PI;
          // Calculer le mouvement horizontal aléatoire naturel
        ship.randomOffset = glm::vec3(
            sin(ship.horizontalPhase1) * ship.horizontalAmp1 + sin(ship.horizontalPhase2) * ship.horizontalAmp2,
            0.0f, // Pas d'oscillation verticale ici (gérée dans le rendu)
            cos(ship.horizontalPhase1 * 0.7f) * ship.horizontalAmp1 + cos(ship.horizontalPhase2 * 1.3f) * ship.horizontalAmp2
        );
    }
    
    // Mettre à jour la caméra en mode pilote
    if (pilotMode) {
        UpdatePilotCamera(camera);
    }
}

void MainScene::Render(Camera& camera, int screenWidth, int screenHeight) {
    if (!initialized) return;

    RenderObjects(camera, screenWidth, screenHeight);
}

void MainScene::RenderUI(GLFWwindow* window, SoundManager& soundManager) {
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

const char* MainScene::GetName() const {
    return "Scène Principale";
}

void MainScene::OnActivate() {
    std::cout << "MainScene activée - Thème spatial" << std::endl;
    // Toujours démarrer l'audio spatial lors de l'activation
    if (ambientSource && zooSound) {
        ambientSource->Stop(); // S'assurer qu'il n'y a pas de son résiduel
        ambientSource->Play(zooSound, true);
        std::cout << "Démarrage de la musique spatial_theme" << std::endl;
    } else {
        std::cout << "MainScene: Impossible de démarrer la musique (source ou son manquant)" << std::endl;
    }
}

void MainScene::OnDeactivate() {
    std::cout << "MainScene désactivée" << std::endl;
    // Arrêter complètement l'audio pour cette scène (au lieu de pause)
    if (ambientSource) {
        std::cout << "MainScene: Source audio existe" << std::endl;
        if (ambientSource->IsPlaying() || ambientSource->IsPaused()) {
            std::cout << "MainScene: Arrêt de la musique..." << std::endl;
            ambientSource->Stop();
            std::cout << "Arrêt de la musique spatial_theme" << std::endl;
        } else {
            std::cout << "MainScene: Musique n'était pas en cours de lecture" << std::endl;
        }
    } else {
        std::cout << "MainScene: Pas de source audio disponible" << std::endl;
    }
}

void MainScene::SetupCameraOverview(Camera& camera) {
    // Position de la caméra pour une vue d'ensemble de la scène
    // Placer la caméra en arrière et en hauteur pour voir :
    // - Le soleil au centre
    // - L'anneau d'astéroïdes (rayon ~60)  
    // - Les vaisseaux français (rayon 180)
    // - La lune (rayon 250)
    
    // Position basée sur l'image fournie
    glm::vec3 overviewPosition = glm::vec3(-229.4f, 131.9f, 435.1f);
    
    // Point vers lequel regarder (centre du système solaire)
    glm::vec3 targetPosition = glm::vec3(-100.0f, 15.0f, -100.0f); // Position du soleil
    
    // Calculer la direction
    glm::vec3 direction = glm::normalize(targetPosition - overviewPosition);
    
    // Mettre à jour la position et l'orientation de la caméra
    camera.Position = overviewPosition;
    camera.Front = direction;
    
    // Calculer les vecteurs Right et Up
    camera.Right = glm::normalize(glm::cross(camera.Front, glm::vec3(0.0f, 1.0f, 0.0f)));
    camera.Up = glm::normalize(glm::cross(camera.Right, camera.Front));    
    // Ajuster le zoom pour une vue plus large
    camera.Zoom = 35.0f; // Vue plus large que les 45° par défaut
    
    std::cout << "Caméra positionnée à (-229.4, 131.9, 435.1) pour vue d'ensemble de la scène spatiale" << std::endl;
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
    }    // Obtenir les shaders depuis le gestionnaire global
    Shader* metalShader = ShaderManager::getInstance().GetMetalShader();
    Shader* texturedShader = ShaderManager::getInstance().GetTexturedShader();
    Shader* sunShader = ShaderManager::getInstance().GetSunShader();
    Shader* currentLightingShader = ShaderManager::getInstance().GetCurrentLightingShader();

    //======== LUNE (orbite autour du soleil, plus éloignée que les astéroïdes) ========
    if (texturedShader && g_uboManager) {
        texturedShader->use();

        // Orbite de la lune autour du soleil (plus éloignée que les astéroïdes)
        float orbitAngle = currentFrame * luneOrbitSpeed;
        float moonX = luneOrbitRadius * cos(orbitAngle);
        float moonZ = luneOrbitRadius * sin(orbitAngle);
        
        // Légère variation verticale pour donner un aspect plus naturel
        float moonY = sin(orbitAngle * 0.7f) * 5.0f;

        glm::mat4 moonModel = glm::mat4(1.0f);
        // Orbiter autour du soleil (lightPosition)
        moonModel = glm::translate(moonModel, glm::vec3(
            lightPosition.x + moonX, 
            lightPosition.y + moonY, 
            lightPosition.z + moonZ
        ));
        // Rotation propre de la lune (synchronisée avec son orbite comme la vraie Lune)
        moonModel = glm::rotate(moonModel, orbitAngle, glm::vec3(0.0f, 1.0f, 0.0f));
        
        g_uboManager->UpdateTransformUBO(moonModel);
        moonSphere->Draw(*texturedShader);
    }

    //======== SOLEIL (utilise son shader spécialisé) ========
    if (sunShader && g_uboManager) {
        sunShader->use();
        sunShader->setFloat("time", currentFrame);

        glm::mat4 sunModel = glm::mat4(1.0f);
        sunModel = glm::translate(sunModel, lightPosition);
        g_uboManager->UpdateTransformUBO(sunModel);
        
        sunSphere->Draw(*sunShader);    }    //======== ANNEAU D'ASTÉROÏDES (utilisent le shader d'éclairage sélectionné) ========
    if (currentLightingShader && g_uboManager && asteroidModel) {
        currentLightingShader->use();
        
        const float baseOrbitRadius = 60.0f;
        const float orbitHeight = 12.0f; // Hauteur de variation augmentée pour plus de relief
        
        for (int i = 0; i < ASTEROID_COUNT; ++i) {
            const AsteroidData& asteroid = asteroids[i];
            
            // Calcul de la position orbitale avec variations plus complexes
            float currentAngle = currentFrame * asteroid.orbitSpeed + asteroid.angleOffset;
            float orbitRadius = baseOrbitRadius + asteroid.radiusOffset;
            
            // Variations verticales plus complexes pour simuler l'épaisseur de la ceinture
            float verticalVariation = sin(currentAngle * 2.5f + asteroid.angleOffset * 3.0f) * orbitHeight * 0.15f;
            verticalVariation += sin(currentAngle * 1.2f + asteroid.angleOffset * 1.7f) * orbitHeight * 0.08f;
            
            // Position de base dans le plan orbital
            glm::vec3 asteroidPosition = glm::vec3(
                orbitRadius * cos(currentAngle),
                verticalVariation,
                orbitRadius * sin(currentAngle)
            );
            
            // Inclinaison variable de l'anneau pour plus de réalisme
            float inclinationAngle = glm::radians(3.0f + sin(asteroid.angleOffset * 2.0f) * 4.0f);
            glm::mat4 inclinationMatrix = glm::rotate(glm::mat4(1.0f), inclinationAngle, 
                                                     glm::vec3(cos(asteroid.angleOffset), 0.0f, sin(asteroid.angleOffset)));
            glm::vec4 inclinedPosition = inclinationMatrix * glm::vec4(asteroidPosition, 1.0f);
            
            // Couleur de l'astéroïde avec légère variation d'intensité basée sur la distance
            glm::vec3 finalColor = asteroid.color;
            float distanceFactor = 1.0f + (asteroid.radiusOffset / 50.0f); // Variation subtile
            finalColor *= distanceFactor;
            currentLightingShader->setVec3("objectColor", finalColor);
            
            // Transformation de l'astéroïde
            glm::mat4 asteroidModel = glm::mat4(1.0f);
            asteroidModel = glm::translate(asteroidModel, glm::vec3(
                lightPosition.x + inclinedPosition.x,
                lightPosition.y + inclinedPosition.y,
                lightPosition.z + inclinedPosition.z
            ));
            
            // Échelle de l'astéroïde avec légère variation temporelle
            float scaleVariation = 1.0f + sin(currentFrame * 0.3f + asteroid.angleOffset * 5.0f) * 0.05f;
            glm::vec3 finalScale = glm::vec3(asteroid.scale * scaleVariation);
            asteroidModel = glm::scale(asteroidModel, finalScale);
            
            // Rotation propre de l'astéroïde plus complexe
            float rotationAngle = currentFrame * asteroid.rotationSpeed + asteroid.angleOffset * 10.0f;
            asteroidModel = glm::rotate(asteroidModel, rotationAngle, asteroid.rotationAxis);
            
            // Rotation supplémentaire pour plus de mouvement
            float secondaryRotation = currentFrame * asteroid.rotationSpeed * 0.3f;
            glm::vec3 secondaryAxis = glm::normalize(glm::vec3(asteroid.rotationAxis.z, asteroid.rotationAxis.x, asteroid.rotationAxis.y));
            asteroidModel = glm::rotate(asteroidModel, secondaryRotation, secondaryAxis);
            
            g_uboManager->UpdateTransformUBO(asteroidModel);
            this->asteroidModel->Draw(*currentLightingShader);        }
          // Afficher des informations de debug occasionnelles
        static int frameCounter = 0;
        frameCounter++;
        if (frameCounter % 300 == 0) { // Toutes les 5 secondes environ à 60 FPS
            std::cout << "Anneau d'astéroïdes : " << ASTEROID_COUNT << " astéroïdes en orbite" << std::endl;
        }
    }
    
    //======== VAISSEAUX FRANÇAIS (utilisent le shader d'éclairage sélectionné) ========
    if (currentLightingShader && g_uboManager && spaceshipModel) {
        currentLightingShader->use();
        
        for (int i = 0; i < SPACESHIP_COUNT; ++i) {
            const SpaceshipData& ship = spaceships[i];            // Position orbitale de base
            float x = ship.orbitRadius * cos(ship.currentAngle);
            float z = ship.orbitRadius * sin(ship.currentAngle);
            
            // === VARIATION DE HAUTEUR NATURELLE ET UNIQUE POUR CHAQUE VAISSEAU ===
            // Utiliser les paramètres individuels pour créer des mouvements complètement différents
            float heightVariation1 = sin(ship.heightPhase1) * ship.heightAmp1;
            float heightVariation2 = sin(ship.heightPhase2) * ship.heightAmp2;
            float heightVariation3 = sin(ship.heightPhase3) * ship.heightAmp3;
            
            // Ajouter des variations complexes basées sur l'angle orbital pour plus de naturel
            float orbitBasedVariation = sin(ship.currentAngle * 0.5f + ship.randomPhase) * 3.0f;
            
            // Combinaison de toutes les oscillations pour un mouvement unique
            float y = heightVariation1 + heightVariation2 + heightVariation3 + orbitBasedVariation;
            
            // === MOUVEMENT HORIZONTAL NATUREL ===
            // Utiliser les oscillations horizontales calculées dans Update()
            glm::vec3 horizontalOffset = glm::vec3(
                ship.randomOffset.x * 0.8f,  // Utiliser l'oscillation horizontale calculée
                0.0f,                         // Pas d'offset vertical supplémentaire ici
                ship.randomOffset.z * 0.8f   // Utiliser l'oscillation horizontale calculée
            );
            
            glm::vec3 basePosition = glm::vec3(x, y, z) + horizontalOffset;
            
            // Position finale (orbite autour du soleil)
            glm::vec3 finalPosition = glm::vec3(
                lightPosition.x + basePosition.x,
                lightPosition.y + basePosition.y,
                lightPosition.z + basePosition.z
            );
            
            // === ORIENTATION CORRECTE : NEZ DANS LA DIRECTION DU MOUVEMENT ===
            // Calculer la direction du mouvement orbital (tangente à l'orbite)
            glm::vec3 movementDirection = glm::vec3(-sin(ship.currentAngle), 0.0f, cos(ship.currentAngle));
            movementDirection = glm::normalize(movementDirection);
            
            // Le "nez" du vaisseau pointe vers l'avant par défaut (axe Z+ dans le modèle)
            // Calculer l'angle de rotation pour orienter le nez dans la direction du mouvement
            float orientationAngle = atan2(movementDirection.x, movementDirection.z);
            
            // Couleur du vaisseau selon le drapeau français
            currentLightingShader->setVec3("objectColor", ship.color);
            
            // Matrice de transformation du vaisseau
            glm::mat4 spaceshipModel_mat = glm::mat4(1.0f);
            spaceshipModel_mat = glm::translate(spaceshipModel_mat, finalPosition);
              // Orientation : rotation pour que le nez suive la trajectoire
            spaceshipModel_mat = glm::rotate(spaceshipModel_mat, orientationAngle, glm::vec3(0.0f, 1.0f, 0.0f));
              // Échelle augmentée pour les vaisseaux pour les rendre plus visibles
            spaceshipModel_mat = glm::scale(spaceshipModel_mat, glm::vec3(0.8f, 0.8f, 0.8f));
            
            // === INCLINAISONS NATURELLES BASÉES SUR LES MOUVEMENTS ===
            // Inclinaison latérale basée sur le mouvement horizontal (comme un avion qui vire)
            float lateralTilt = (ship.randomOffset.x / ship.horizontalAmp1) * 0.2f; // Inclinaison proportionnelle au mouvement
            spaceshipModel_mat = glm::rotate(spaceshipModel_mat, lateralTilt, glm::vec3(0.0f, 0.0f, 1.0f));
            
            // Inclinaison longitudinale basée sur la variation de hauteur (comme un avion qui monte/descend)
            float verticalTilt = (heightVariation1 / ship.heightAmp1) * 0.15f;
            spaceshipModel_mat = glm::rotate(spaceshipModel_mat, verticalTilt, glm::vec3(1.0f, 0.0f, 0.0f));
            
            // Légère rotation de roulis pour plus de dynamisme
            float rollAngle = sin(ship.randomPhase * 0.5f) * 0.08f;
            spaceshipModel_mat = glm::rotate(spaceshipModel_mat, rollAngle, glm::vec3(0.0f, 1.0f, 0.0f));
            
            g_uboManager->UpdateTransformUBO(spaceshipModel_mat);
            this->spaceshipModel->Draw(*currentLightingShader);
        }
        
        // Debug occasionnel pour les vaisseaux
        static int spaceshipFrameCounter = 0;
        spaceshipFrameCounter++;
        if (spaceshipFrameCounter % 400 == 0) {
            std::cout << "Vaisseaux français en formation : Bleu, Blanc, Rouge" << std::endl;
        }
    }
}

void MainScene::Cleanup() {
    if (ambientSource) {
        ambientSource->Stop();
    }    // Les modèles 3D se nettoient automatiquement avec les smart pointers
    asteroidModel.reset(); // Un seul modèle d'astéroïde maintenant
    spaceshipModel.reset(); // Modèle des vaisseaux français

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

bool MainScene::ChangeSceneSound(const std::string& soundName, SoundManager& soundManager) {
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
        std::cout << "MainScene: Son changé vers " << soundName << " (reprise automatique)" << std::endl;
    } else {
        std::cout << "MainScene: Son changé vers " << soundName << std::endl;
    }
    
    return true;
}

std::string MainScene::GetCurrentSoundName() const {
    return currentSoundName;
}

void MainScene::InitializeAsteroidRing() {
    // Couleurs réalistes d'astéroïdes étendues (basées sur leur composition minéralogique)
    std::vector<glm::vec3> asteroidColors = {
        // Astéroïdes de type S (silicatés) - plus clairs
        glm::vec3(0.55f, 0.45f, 0.35f), // Beige rocheuse (chondrite ordinaire)
        glm::vec3(0.50f, 0.40f, 0.30f), // Terre de Sienne (olivine + pyroxène)
        glm::vec3(0.60f, 0.50f, 0.40f), // Beige clair (plagioclase)
        glm::vec3(0.45f, 0.38f, 0.28f), // Ocre (composition mixte)
        
        // Astéroïdes de type C (carbonés) - plus sombres
        glm::vec3(0.25f, 0.20f, 0.15f), // Brun très foncé (carbone + hydrates)
        glm::vec3(0.30f, 0.25f, 0.20f), // Brun-gris foncé
        glm::vec3(0.35f, 0.28f, 0.22f), // Gris-brun (argiles)
        glm::vec3(0.28f, 0.22f, 0.18f), // Anthracite
        
        // Astéroïdes de type M (métalliques) - reflets métalliques
        glm::vec3(0.45f, 0.45f, 0.45f), // Gris métallique (fer-nickel)
        glm::vec3(0.50f, 0.48f, 0.46f), // Acier patiné
        glm::vec3(0.42f, 0.40f, 0.38f), // Graphite métallique
        glm::vec3(0.48f, 0.46f, 0.44f), // Fer oxydé
        
        // Astéroïdes riches en mineraux spécifiques
        glm::vec3(0.40f, 0.32f, 0.24f), // Rouille (hématite)
        glm::vec3(0.52f, 0.42f, 0.32f), // Grès (quartz + feldspath)
        glm::vec3(0.38f, 0.35f, 0.30f), // Basalte altéré
        glm::vec3(0.46f, 0.40f, 0.34f), // Schiste
        
        // Variations supplémentaires pour plus de diversité
        glm::vec3(0.36f, 0.30f, 0.24f), // Terre d'ombre
        glm::vec3(0.58f, 0.48f, 0.38f), // Sable désertique
        glm::vec3(0.33f, 0.27f, 0.21f), // Tourbe
        glm::vec3(0.51f, 0.43f, 0.35f), // Argile cuite
        
        // Astéroïdes avec traces de métaux précieux ou rares
        glm::vec3(0.44f, 0.41f, 0.36f), // Bronze patiné
        glm::vec3(0.39f, 0.36f, 0.31f), // Cuivre oxydé
        glm::vec3(0.47f, 0.43f, 0.37f), // Laiton terni
        glm::vec3(0.41f, 0.37f, 0.32f)  // Fer-nickel avec traces
    };

    // Initialiser le générateur de nombres aléatoires
    srand(static_cast<unsigned int>(42)); // Graine fixe pour des résultats reproductibles

    for (int i = 0; i < ASTEROID_COUNT; ++i) {
        AsteroidData& asteroid = asteroids[i];
        
        // Répartition plus dense avec quelques variations pour éviter la régularité
        float baseAngle = (2.0f * M_PI * i) / ASTEROID_COUNT;
        float angleVariation = (rand() % 20 - 10) * 0.01f; // ±0.1 radian de variation
        asteroid.angleOffset = baseAngle + angleVariation;
        
        // Variation plus importante du rayon orbital pour créer des "groupes" d'astéroïdes
        if (i % 8 == 0) {
            // Quelques astéroïdes plus éloignés pour simuler des "familles" d'astéroïdes
            asteroid.radiusOffset = 8.0f + (rand() % 8); // Entre +8 et +15
        } else if (i % 12 == 0) {
            // Quelques astéroïdes plus proches
            asteroid.radiusOffset = -8.0f - (rand() % 6); // Entre -8 et -13
        } else {
            // Majorité dans la ceinture principale
            asteroid.radiusOffset = -3.0f + (rand() % 7); // Entre -3 et +3
        }
        
        // Échelles plus variées avec quelques "gros" astéroïdes occasionnels
        float baseScale;
        if (i % 15 == 0) {
            // Gros astéroïdes occasionnels
            baseScale = 0.08f + (rand() % 4) * 0.02f; // Entre 0.08 et 0.14
        } else if (i % 7 == 0) {
            // Astéroïdes moyens
            baseScale = 0.04f + (rand() % 4) * 0.01f; // Entre 0.04 et 0.07
        } else {
            // Petits astéroïdes (majoritaires)
            baseScale = 0.015f + (rand() % 4) * 0.005f; // Entre 0.015 et 0.03
        }
        asteroid.scale = baseScale;
        
        // Vitesses de rotation plus variées
        asteroid.rotationSpeed = 1.0f + (rand() % 15) * 0.5f; // Entre 1.0 et 8.5
        
        // Axes de rotation aléatoires mais plus naturels
        asteroid.rotationAxis = glm::normalize(glm::vec3(
            (rand() % 200 - 100) / 100.0f,  // -1.0 à 1.0
            (rand() % 200 - 100) / 100.0f,
            (rand() % 200 - 100) / 100.0f
        ));
        
        // Couleur réaliste avec plus de variété
        asteroid.color = asteroidColors[i % asteroidColors.size()];
        
        // Vitesse orbitale variée avec légère tendance selon la distance
        float baseOrbitSpeed = 0.4f + (rand() % 6) * 0.05f; // Entre 0.4 et 0.65
        // Les astéroïdes plus éloignés vont légèrement plus lentement (3ème loi de Kepler simplifiée)
        if (asteroid.radiusOffset > 5.0f) {
            baseOrbitSpeed *= 0.9f;
        } else if (asteroid.radiusOffset < -5.0f) {
            baseOrbitSpeed *= 1.1f;
        }
        asteroid.orbitSpeed = baseOrbitSpeed;
    }
      std::cout << "Anneau d'astéroïdes initialisé avec " << ASTEROID_COUNT << " astéroïdes" << std::endl;
    std::cout << "Palette de couleurs étendue : " << asteroidColors.size() << " variations réalistes" << std::endl;
}

void MainScene::InitializeSpaceships() {
    // Couleurs du drapeau français : Bleu, Blanc, Rouge
    glm::vec3 frenchColors[SPACESHIP_COUNT] = {
        glm::vec3(0.0f, 0.2f, 0.8f),    // Bleu France
        glm::vec3(0.9f, 0.9f, 0.9f),    // Blanc
        glm::vec3(0.8f, 0.1f, 0.1f)     // Rouge France
    };
    
    // Initialiser le générateur avec une graine différente
    srand(static_cast<unsigned int>(123));
    
    for (int i = 0; i < SPACESHIP_COUNT; ++i) {
        SpaceshipData& ship = spaceships[i];
        
        // Couleur selon le drapeau français
        ship.color = frenchColors[i];
          // Angles de départ côte à côte avec un petit espacement
        ship.angleOffset = (i - 1) * 0.15f; // Environ 8.6° d'espacement entre les vaisseaux
        
        // Tous orbitent au même rayon (entre les astéroïdes et la lune)
        ship.orbitRadius = 180.0f; // Entre les astéroïdes (~60) et la lune (250)
          // Vitesse orbitale synchronisée mais légèrement variée
        ship.orbitSpeed = 0.25f + (i * 0.025f); // Vitesses légèrement différentes : 0.25, 0.275, 0.3 (réduit de moitié)
        
        // Angle initial
        ship.currentAngle = ship.angleOffset;
        
        // Mouvement aléatoire horizontal léger pour plus de réalisme
        ship.randomOffset = glm::vec3(
            ((rand() % 200) - 100) / 1000.0f,  // ±0.1 unités de décalage horizontal
            0.0f,                               // Pas d'offset vertical initial
            ((rand() % 200) - 100) / 1000.0f   // ±0.1 unités de décalage horizontal
        );
          // Phase aléatoire différente pour chaque vaisseau pour des variations de hauteur uniques
        ship.randomPhase = (rand() % 628) / 100.0f + (i * 2.1f); // 0 à 2π + décalage par vaisseau
        
        // === PARAMÈTRES INDIVIDUELS POUR MOUVEMENTS VERTICAUX NATURELS ===
        // Chaque vaisseau a ses propres fréquences, amplitudes et phases pour un comportement unique
        
        // Oscillation principale (lente et ample)
        ship.heightFreq1 = 0.8f + ((rand() % 100) / 100.0f) * 0.6f;    // 0.8 à 1.4
        ship.heightAmp1 = 6.0f + ((rand() % 100) / 100.0f) * 8.0f;     // 6 à 14 unités
        ship.heightPhase1 = ((rand() % 628) / 100.0f);                  // 0 à 2π
        
        // Oscillation secondaire (moyenne)
        ship.heightFreq2 = 1.2f + ((rand() % 100) / 100.0f) * 1.0f;    // 1.2 à 2.2
        ship.heightAmp2 = 2.0f + ((rand() % 100) / 100.0f) * 4.0f;     // 2 à 6 unités
        ship.heightPhase2 = ((rand() % 628) / 100.0f);                  // 0 à 2π
        
        // Oscillation rapide (petite et vive)
        ship.heightFreq3 = 2.5f + ((rand() % 100) / 100.0f) * 2.0f;    // 2.5 à 4.5
        ship.heightAmp3 = 0.5f + ((rand() % 100) / 100.0f) * 2.0f;     // 0.5 à 2.5 unités
        ship.heightPhase3 = ((rand() % 628) / 100.0f);                  // 0 à 2π
        
        // === PARAMÈTRES INDIVIDUELS POUR MOUVEMENTS HORIZONTAUX NATURELS ===
        
        // Oscillation horizontale lente (déviation générale)
        ship.horizontalFreq1 = 0.3f + ((rand() % 100) / 100.0f) * 0.4f; // 0.3 à 0.7
        ship.horizontalAmp1 = 1.0f + ((rand() % 100) / 100.0f) * 2.0f;  // 1 à 3 unités
        ship.horizontalPhase1 = ((rand() % 628) / 100.0f);               // 0 à 2π
        
        // Oscillation horizontale rapide (vibrations)
        ship.horizontalFreq2 = 1.5f + ((rand() % 100) / 100.0f) * 1.0f; // 1.5 à 2.5
        ship.horizontalAmp2 = 0.2f + ((rand() % 100) / 100.0f) * 0.6f;  // 0.2 à 0.8 unités
        ship.horizontalPhase2 = ((rand() % 628) / 100.0f);               // 0 à 2π
    }
    
    std::cout << "Vaisseaux français initialisés : Bleu, Blanc, Rouge" << std::endl;
}

//======== MÉTHODES POUR LE MODE PILOTE ========

void MainScene::TogglePilotMode() {
    pilotMode = !pilotMode;
    if (pilotMode) {
        // Changer de vaisseau à chaque activation (rotation entre les 3 vaisseaux)
        currentSpaceshipIndex = (currentSpaceshipIndex + 1) % SPACESHIP_COUNT;
        // Initialiser la position précédente
        lastSpaceshipPosition = GetSpaceshipPosition(currentSpaceshipIndex);
    }
}

void MainScene::UpdatePilotCamera(Camera& camera) {
    if (!pilotMode || currentSpaceshipIndex < 0 || currentSpaceshipIndex >= SPACESHIP_COUNT) {
        return;
    }
    
    // Obtenir la position actuelle du vaisseau suivi
    glm::vec3 spaceshipPosition = GetSpaceshipPosition(currentSpaceshipIndex);
    
    // Positionner la caméra derrière et au-dessus du vaisseau
    glm::vec3 offset = glm::vec3(-15.0f, 8.0f, -20.0f); // Derrière, légèrement au-dessus et décalé
    
    // Appliquer l'offset à la position du vaisseau
    camera.Position = spaceshipPosition + offset;
    
    // En mode pilote, on ne modifie PAS l'orientation de la caméra
    // L'utilisateur garde le contrôle avec la souris
    // Les vecteurs Front, Right, Up sont gérés par les contrôles de la caméra normale
    
    // Sauvegarder la position pour le prochain frame
    lastSpaceshipPosition = spaceshipPosition;
}

glm::vec3 MainScene::GetSpaceshipPosition(int index) const {
    if (index < 0 || index >= SPACESHIP_COUNT) {
        return glm::vec3(0.0f);
    }
    
    const SpaceshipData& ship = spaceships[index];
    
    // Recalculer la position exacte du vaisseau (même logique que dans RenderObjects)
    float x = ship.orbitRadius * cos(ship.currentAngle);
    float z = ship.orbitRadius * sin(ship.currentAngle);
    
    // Variation de hauteur naturelle
    float heightVariation1 = sin(ship.heightPhase1) * ship.heightAmp1;
    float heightVariation2 = sin(ship.heightPhase2) * ship.heightAmp2;
    float heightVariation3 = sin(ship.heightPhase3) * ship.heightAmp3;
    float orbitBasedVariation = sin(ship.currentAngle * 0.5f + ship.randomPhase) * 3.0f;
    float y = heightVariation1 + heightVariation2 + heightVariation3 + orbitBasedVariation;
    
    // Mouvement horizontal naturel
    glm::vec3 horizontalOffset = glm::vec3(
        ship.randomOffset.x * 0.8f,
        0.0f,
        ship.randomOffset.z * 0.8f
    );
    
    glm::vec3 basePosition = glm::vec3(x, y, z) + horizontalOffset;
    
    // Position finale (orbite autour du soleil)
    return glm::vec3(
        lightPosition.x + basePosition.x,
        lightPosition.y + basePosition.y,
        lightPosition.z + basePosition.z
    );
}
