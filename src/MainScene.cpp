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
        
        // Charger un seul modèle d'astéroïde (réutilisé pour tout l'anneau)
        asteroidModel = std::make_unique<Model>("../models/astroid.obj");
          // Initialiser l'anneau d'astéroïdes avec des propriétés variées
        InitializeAsteroidRing();
        
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
    }    //======== LUNE (orbite autour du soleil, plus éloignée que les astéroïdes) ========
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
}

void MainScene::Cleanup() {
    if (ambientSource) {
        ambientSource->Stop();
    }    // Les modèles 3D se nettoient automatiquement avec les smart pointers
    myModel.reset();
    asteroidModel.reset(); // Un seul modèle d'astéroïde maintenant

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
