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
    : lightPosition(-50.0f, 20.0f, -50.0f),
      lightColor(1.0f, 1.0f, 0.8f),
      lightRadius(5.0f),
      rotationSpeed(0.0f),
      currentRotation(0.0f),
      initialized(false) {}

LightScene::~LightScene() { Cleanup(); }

bool LightScene::Initialize(Camera& camera, SoundManager& soundManager) {
    if (initialized) return true;
    if (!LoadShaders()) return false;
    if (!CreateLightSphere()) return false;
    if (!LoadModels()) return false;
    LoadAudio(soundManager);
    currentSkyboxType = SkyboxManager::SkyboxType::SPACE;
    skybox = std::make_unique<Skybox>(SkyboxManager::GetSkyboxFaces(currentSkyboxType));
    initialized = true;
    return true;
}

bool LightScene::LoadShaders() {
    return true;
}

bool LightScene::CreateLightSphere() {
    try {
        sunSphere = std::make_unique<Sphere>("", sunRadius, 64, 32);
        moonSphere = std::make_unique<Sphere>("../textures/spherical_moon_texture.jpg", moonRadius, 36, 18);
        lightSphere = std::make_unique<Sphere>("", lightRadius, 32, 16);
        testSphere = std::make_unique<Sphere>("", 3.0f, 32, 16);
        return true;
    } catch (...) { return false; }
}

void LightScene::Update(float deltaTime, GLFWwindow*, Camera&, SoundManager&) {
    if (!initialized) return;
    moonCurrentAngle += moonOrbitSpeed * deltaTime;
    if (moonCurrentAngle > 2.0f * M_PI) moonCurrentAngle -= 2.0f * M_PI;
    for (int i = 0; i < ASTEROID_COUNT; ++i) {
        AsteroidData& asteroid = asteroids[i];
        asteroid.currentAngle += asteroid.orbitSpeed * deltaTime;
        if (asteroid.currentAngle > 2.0f * M_PI) asteroid.currentAngle -= 2.0f * M_PI;
    }
    for (int i = 0; i < SPACESHIP_COUNT; ++i) {
        SpaceshipData& ship = spaceships[i];
        ship.currentAngle += ship.orbitSpeed * deltaTime;
        if (ship.currentAngle > 2.0f * M_PI) ship.currentAngle -= 2.0f * M_PI;
        ship.randomPhase += deltaTime * 2.0f;
        if (ship.randomPhase > 2.0f * M_PI) ship.randomPhase -= 2.0f * M_PI;
        ship.heightPhase1 += deltaTime * ship.heightFreq1;
        ship.heightPhase2 += deltaTime * ship.heightFreq2;
        ship.heightPhase3 += deltaTime * ship.heightFreq3;
        ship.horizontalPhase1 += deltaTime * ship.horizontalFreq1;
        ship.horizontalPhase2 += deltaTime * ship.horizontalFreq2;
    }
    globalTime += deltaTime;
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
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)screenWidth / screenHeight, 0.1f, 1000.0f);
    glm::mat4 view = glm::mat4(glm::mat3(camera.GetViewMatrix()));
    if (skybox) skybox->Render(view, projection);
    RenderSun(camera, screenWidth, screenHeight);
    RenderAsteroidRing(camera, screenWidth, screenHeight);
    RenderSpaceships(camera, screenWidth, screenHeight);
    RenderMoon(camera, screenWidth, screenHeight);
    RenderLight(camera, screenWidth, screenHeight);
    RenderStations(camera, screenWidth, screenHeight);
    RenderComets(camera, screenWidth, screenHeight);
    RenderDebris(camera, screenWidth, screenHeight);
    RenderPortals(camera, screenWidth, screenHeight);
    RenderSatellites(camera, screenWidth, screenHeight);
    RenderParticleClouds(camera, screenWidth, screenHeight);
}

void LightScene::RenderLight(Camera& camera, int screenWidth, int screenHeight) {
    Shader* sunShader = ShaderManager::getInstance().GetSunShader();
    if (!sunShader || !g_uboManager) return;
    sunShader->use();
    sunShader->setFloat("time", static_cast<float>(glfwGetTime()));
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, lightPosition);
    g_uboManager->UpdateTransformUBO(model);
    lightSphere->Draw(*sunShader);
    Shader* currentLightingShader = ShaderManager::getInstance().GetCurrentLightingShader();
    if (currentLightingShader && testSphere) {
        currentLightingShader->use();
        currentLightingShader->setVec3("objectColor", glm::vec3(0.8f, 0.3f, 0.1f));
        glm::mat4 testModel = glm::mat4(1.0f);
        testModel = glm::translate(testModel, lightPosition + glm::vec3(10.0f, 0.0f, 0.0f));
        g_uboManager->UpdateTransformUBO(testModel);
        testSphere->Draw(*currentLightingShader);
    }
}

void LightScene::RenderUI(GLFWwindow* window, SoundManager& soundManager) {
    if (!initialized) return;
    UIHelpers::RenderKeyboardUI(window);
    UIHelpers::RenderAudioUI(window, soundManager, ambientSource, zooSound, currentSoundName,
        [this, &soundManager](const std::string& soundName) -> bool {
            return this->ChangeSceneSound(soundName, soundManager);
        });
    UIHelpers::RenderMainControlsUI(currentSkyboxType,
        [this](SkyboxManager::SkyboxType type) { ChangeSkybox(type); }
    );
}

const char* LightScene::GetName() const { return "Scène de Lumière"; }

void LightScene::OnActivate() {
    if (ambientSource && zooSound) {
        ambientSource->Stop();
        ambientSource->Play(zooSound, true);
    }
}

void LightScene::OnDeactivate() {
    if (ambientSource) ambientSource->Stop();
}

void LightScene::Cleanup() {
    lightSphere.reset();
    testSphere.reset();
    initialized = false;
}

bool LightScene::LoadAudio(SoundManager& soundManager) {
    if (!soundManager.IsInitialized()) return false;
    std::string soundName = "Zoo";
    zooSound = soundManager.GetSound(soundName);
    if (!zooSound) {
        soundName = "spatial_theme";
        zooSound = soundManager.GetSound(soundName);
    }
    currentSoundName = soundName;
    ambientSource = soundManager.CreateAudioSource();
    if (ambientSource) {
        ambientSource->SetPosition({0.0f, 0.0f, 0.0f});
        ambientSource->SetVolume(0.3f);
    }
    return (zooSound && ambientSource);
}

void LightScene::ChangeSkybox(SkyboxManager::SkyboxType newType) {
    if (newType != currentSkyboxType) {
        currentSkyboxType = newType;
        skybox = std::make_unique<Skybox>(SkyboxManager::GetSkyboxFaces(currentSkyboxType));
    }
}

bool LightScene::ChangeSceneSound(const std::string& soundName, SoundManager& soundManager) {
    auto newSound = soundManager.GetSound(soundName);
    if (!newSound) return false;
    bool wasPlaying = ambientSource && ambientSource->IsPlaying();
    bool wasPaused = ambientSource && ambientSource->IsPaused();
    if (ambientSource && (wasPlaying || wasPaused)) ambientSource->Stop();
    zooSound = newSound;
    currentSoundName = soundName;
    if (ambientSource && wasPlaying) ambientSource->Play(zooSound, true);
    return true;
}

std::string LightScene::GetCurrentSoundName() const { return currentSoundName; }

bool LightScene::LoadModels() {
    try {
        asteroidModel = std::make_unique<Model>("../models/astroid.obj");
        spaceshipModel = std::make_unique<Model>("../models/map-bump.obj");
        InitializeAsteroidRing();
        InitializeSpaceships();
        InitializeStations();
        InitializeComets();
        InitializeDebris();
        InitializePortals();
        InitializeSatellites();
        InitializeParticleClouds();
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
    for (int i = 0; i < ASTEROID_COUNT; ++i) {
        AsteroidData& asteroid = asteroids[i];
        asteroid.angleOffset = (2.0f * M_PI * i / ASTEROID_COUNT) + 
                              ((rand() % 100) / 100.0f - 0.5f) * 0.5f;
        float baseRadius = 150.0f + (100.0f * i / ASTEROID_COUNT);
        asteroid.radiusOffset = baseRadius + ((rand() % 100) / 100.0f - 0.5f) * 30.0f;
        asteroid.scale = 8.0f + ((rand() % 100) / 100.0f) * 12.0f;
        asteroid.rotationSpeed = 0.5f + ((rand() % 100) / 100.0f) * 2.0f;
        if (rand() % 2) asteroid.rotationSpeed *= -1;
        asteroid.rotationAxis = glm::normalize(glm::vec3(
            (rand() % 100) / 100.0f - 0.5f,
            (rand() % 100) / 100.0f - 0.5f,
            (rand() % 100) / 100.0f - 0.5f
        ));
        float colorVariation = (rand() % 100) / 100.0f;
        asteroid.color = glm::mix(
            glm::vec3(0.4f, 0.3f, 0.2f),
            glm::vec3(0.6f, 0.5f, 0.4f),
            colorVariation
        );
        asteroid.orbitSpeed = 0.3f + ((rand() % 100) / 100.0f) * 0.4f;
        asteroid.currentAngle = asteroid.angleOffset;
    }
}

void LightScene::InitializeSpaceships() {
    for (int i = 0; i < SPACESHIP_COUNT; ++i) {
        SpaceshipData& ship = spaceships[i];
        if (i % 5 == 0) ship.color = glm::vec3(0.2f, 0.4f, 1.0f);
        else if (i % 5 == 1) ship.color = glm::vec3(1.0f, 0.8f, 0.2f);
        else if (i % 5 == 2) ship.color = glm::vec3(0.8f, 0.2f, 0.2f);
        else if (i % 5 == 3) ship.color = glm::vec3(0.2f, 0.8f, 0.2f);
        else ship.color = glm::vec3(0.9f, 0.9f, 0.9f);
        ship.angleOffset = (2.0f * M_PI * i / SPACESHIP_COUNT) + 
                          ((rand() % 100) / 100.0f - 0.5f) * 0.3f;
        if (i < SPACESHIP_COUNT / 3) {
            ship.orbitRadius = 300.0f + ((rand() % 100) / 100.0f) * 50.0f;
        } else if (i < 2 * SPACESHIP_COUNT / 3) {
            ship.orbitRadius = 400.0f + ((rand() % 100) / 100.0f) * 50.0f;
        } else {
            ship.orbitRadius = 500.0f + ((rand() % 100) / 100.0f) * 50.0f;
        }
        ship.orbitSpeed = 200.0f / ship.orbitRadius;
        ship.currentAngle = ship.angleOffset;
        ship.randomOffset = glm::vec3(
            ((rand() % 100) / 100.0f - 0.5f) * 10.0f,
            ((rand() % 100) / 100.0f - 0.5f) * 10.0f,
            ((rand() % 100) / 100.0f - 0.5f) * 10.0f
        );
        ship.randomPhase = (rand() % 100) / 100.0f * 2.0f * M_PI;
        ship.heightFreq1 = 0.5f + ((rand() % 100) / 100.0f) * 1.0f;
        ship.heightFreq2 = 1.0f + ((rand() % 100) / 100.0f) * 1.5f;
        ship.heightFreq3 = 0.3f + ((rand() % 100) / 100.0f) * 0.7f;
        ship.heightAmp1 = 5.0f + ((rand() % 100) / 100.0f) * 10.0f;
        ship.heightAmp2 = 3.0f + ((rand() % 100) / 100.0f) * 6.0f;
        ship.heightAmp3 = 2.0f + ((rand() % 100) / 100.0f) * 4.0f;
        ship.heightPhase1 = (rand() % 100) / 100.0f * 2.0f * M_PI;
        ship.heightPhase2 = (rand() % 100) / 100.0f * 2.0f * M_PI;
        ship.heightPhase3 = (rand() % 100) / 100.0f * 2.0f * M_PI;
        ship.horizontalFreq1 = 0.8f + ((rand() % 100) / 100.0f) * 1.2f;
        ship.horizontalFreq2 = 1.2f + ((rand() % 100) / 100.0f) * 1.8f;
        ship.horizontalAmp1 = 8.0f + ((rand() % 100) / 100.0f) * 12.0f;
        ship.horizontalAmp2 = 5.0f + ((rand() % 100) / 100.0f) * 8.0f;
        ship.horizontalPhase1 = (rand() % 100) / 100.0f * 2.0f * M_PI;
        ship.horizontalPhase2 = (rand() % 100) / 100.0f * 2.0f * M_PI;
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
        float x = asteroid.radiusOffset * cos(asteroid.currentAngle);
        float z = asteroid.radiusOffset * sin(asteroid.currentAngle);
        float y = sin(asteroid.currentAngle * 3.0f) * 10.0f;
        glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(x, y, z));
        model = glm::rotate(model, 
                           asteroid.rotationSpeed * static_cast<float>(glfwGetTime()), 
                           asteroid.rotationAxis);
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
        float baseX = ship.orbitRadius * cos(ship.currentAngle);
        float baseZ = ship.orbitRadius * sin(ship.currentAngle);
        float heightOffset = ship.heightAmp1 * sin(ship.heightPhase1) +
                           ship.heightAmp2 * sin(ship.heightPhase2) +
                           ship.heightAmp3 * sin(ship.heightPhase3);
        float horizontalOffset1 = ship.horizontalAmp1 * sin(ship.horizontalPhase1);
        float horizontalOffset2 = ship.horizontalAmp2 * sin(ship.horizontalPhase2);
        glm::vec3 finalPos = glm::vec3(
            baseX + horizontalOffset1 * cos(ship.currentAngle + M_PI/2) + ship.randomOffset.x,
            heightOffset + ship.randomOffset.y,
            baseZ + horizontalOffset1 * sin(ship.currentAngle + M_PI/2) + ship.randomOffset.z
        );
        glm::mat4 model = glm::translate(glm::mat4(1.0f), finalPos);
        glm::vec3 direction = glm::normalize(-finalPos);
        float rotationAngle = atan2(direction.x, direction.z);
        model = glm::rotate(model, rotationAngle, glm::vec3(0.0f, 1.0f, 0.0f));
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
    float x = moonOrbitRadius * cos(moonCurrentAngle);
    float z = moonOrbitRadius * sin(moonCurrentAngle);
    float y = sin(moonCurrentAngle * 0.5f) * 20.0f;
    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(x, y, z));
    model = glm::rotate(model, 
                       moonSelfRotSpeed * static_cast<float>(glfwGetTime()), 
                       glm::vec3(0.0f, 1.0f, 0.1f));
    model = glm::scale(model, glm::vec3(moonRadius));
    glm::mat4 view = camera.GetViewMatrix();
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), 
                                          (float)screenWidth / screenHeight, 0.1f, 1000.0f);
    shader->setMat4("model", model);
    shader->setMat4("view", view);
    shader->setMat4("projection", projection);
    shader->setVec3("objectColor", glm::vec3(0.9f, 0.9f, 0.8f));
    moonSphere->Draw(*shader);
}

void LightScene::InitializeStations() {
    for (int i = 0; i < STATION_COUNT; ++i) {
        SpaceStation& station = stations[i];
        float angle = (2.0f * M_PI * i / STATION_COUNT);
        float radius = 350.0f + (rand() % 100);
        station.position = glm::vec3(radius * cos(angle), 
                                   ((rand() % 100) - 50) * 2.0f, 
                                   radius * sin(angle));
        station.rotationSpeed = 0.5f + (rand() % 100) / 200.0f;
        station.currentRotation = (rand() % 360) * M_PI / 180.0f;
        station.rotationAxis = glm::normalize(glm::vec3(
            (rand() % 100) / 100.0f - 0.5f,
            1.0f,
            (rand() % 100) / 100.0f - 0.5f
        ));
        station.scale = 5.0f + (rand() % 100) / 50.0f;
        station.color = glm::vec3(0.7f + (rand() % 30) / 100.0f, 
                                 0.7f + (rand() % 30) / 100.0f, 
                                 0.9f);
        station.orbitAngle = angle;
        station.orbitRadius = radius;
        station.orbitSpeed = 0.1f + (rand() % 50) / 500.0f;
        station.turretRotation = 0.0f;
        station.turretSpeed = 2.0f + (rand() % 100) / 50.0f;
    }
}

void LightScene::InitializeComets() {
    comets.clear();
    for (int i = 0; i < COMET_COUNT; ++i) {
        Comet comet;
        float angle = (rand() % 360) * M_PI / 180.0f;
        float distance = 800.0f + (rand() % 400);
        comet.position = glm::vec3(distance * cos(angle), 
                                  ((rand() % 200) - 100) * 3.0f, 
                                  distance * sin(angle));
        glm::vec3 direction = glm::normalize(-comet.position);
        direction += glm::vec3((rand() % 100) / 200.0f - 0.25f,
                              (rand() % 100) / 200.0f - 0.25f,
                              (rand() % 100) / 200.0f - 0.25f);
        comet.velocity = direction * (30.0f + (rand() % 40));
        comet.trailLength = 20 + (rand() % 30);
        comet.trailPositions.clear();
        for (int j = 0; j < comet.trailLength; ++j) {
            comet.trailPositions.push_back(comet.position);
        }
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
    debris.clear();
    for (int i = 0; i < DEBRIS_COUNT; ++i) {
        SpaceDebris d;
        float radius = 100.0f + (rand() % 600);
        float theta = (rand() % 360) * M_PI / 180.0f;
        float phi = (rand() % 180) * M_PI / 180.0f;
        d.position = glm::vec3(radius * sin(phi) * cos(theta),
                              radius * cos(phi),
                              radius * sin(phi) * sin(theta));
        d.velocity = glm::vec3((rand() % 100) / 50.0f - 1.0f,
                              (rand() % 100) / 50.0f - 1.0f,
                              (rand() % 100) / 50.0f - 1.0f) * 5.0f;
        d.angularVelocity = glm::vec3((rand() % 100) / 25.0f - 2.0f,
                                     (rand() % 100) / 25.0f - 2.0f,
                                     (rand() % 100) / 25.0f - 2.0f);
        d.rotation = glm::vec3(0.0f);
        d.scale = 0.5f + (rand() % 100) / 100.0f;
        d.color = glm::vec3(0.3f + (rand() % 40) / 100.0f,
                           0.3f + (rand() % 40) / 100.0f,
                           0.3f + (rand() % 40) / 100.0f);
        d.maxLifetime = 60.0f + (rand() % 120);
        d.lifetime = d.maxLifetime;
        debris.push_back(d);
    }
}

void LightScene::InitializePortals() {
    for (int i = 0; i < PORTAL_COUNT; ++i) {
        EnergyPortal& portal = portals[i];
        float angle = (2.0f * M_PI * i / PORTAL_COUNT);
        float radius = 600.0f + (rand() % 200);
        portal.position = glm::vec3(radius * cos(angle),
                                   ((rand() % 100) - 50) * 4.0f,
                                   radius * sin(angle));
        portal.rotationSpeed = 1.0f + (rand() % 100) / 50.0f;
        portal.currentRotation = (rand() % 360) * M_PI / 180.0f;
        portal.pulseIntensity = 0.5f + (rand() % 50) / 100.0f;
        portal.pulseSpeed = 2.0f + (rand() % 100) / 50.0f;
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
    for (int i = 0; i < SATELLITE_COUNT; ++i) {
        Satellite& sat = satellites[i];
        int layer = i / 10;
        int pos = i % 10;
        sat.orbitRadius = 250.0f + layer * 50.0f;
        sat.orbitSpeed = 0.8f - layer * 0.1f;
        sat.currentAngle = (2.0f * M_PI * pos / 10) + layer * 0.3f;
        sat.basePosition = glm::vec3(0.0f, layer * 20.0f - 40.0f, 0.0f);
        sat.antennaRotation = glm::vec3(0.0f);
        sat.antennaSpeed = 1.0f + (rand() % 100) / 100.0f;
        sat.isActive = (rand() % 10) > 2;
        sat.color = sat.isActive ? 
                   glm::vec3(0.2f, 1.0f, 0.3f) :
                   glm::vec3(0.8f, 0.2f, 0.2f);
        sat.signalPulse = (rand() % 360) * M_PI / 180.0f;
    }
}

void LightScene::InitializeParticleClouds() {
    particleClouds.clear();
    for (int i = 0; i < PARTICLE_CLOUD_COUNT; ++i) {
        ParticleCloud cloud;
        float angle = (rand() % 360) * M_PI / 180.0f;
        float radius = 400.0f + (rand() % 300);
        cloud.center = glm::vec3(radius * cos(angle),
                                ((rand() % 200) - 100) * 2.0f,
                                radius * sin(angle));
        cloud.radius = 30.0f + (rand() % 50);
        cloud.rotationSpeed = 0.5f + (rand() % 100) / 200.0f;
        cloud.currentRotation = 0.0f;
        cloud.color = glm::vec3(0.5f + (rand() % 50) / 100.0f,
                               0.2f + (rand() % 60) / 100.0f,
                               0.8f + (rand() % 20) / 100.0f);
        cloud.intensity = 0.3f + (rand() % 70) / 100.0f;
        cloud.pulseSpeed = 1.0f + (rand() % 100) / 100.0f;
        int particleCount = 50 + (rand() % 100);
        cloud.particlePositions.clear();
        cloud.particleVelocities.clear();
        for (int j = 0; j < particleCount; ++j) {
            float r = cloud.radius * pow((rand() % 1000) / 1000.0f, 1.0f/3.0f);
            float theta = (rand() % 360) * M_PI / 180.0f;
            float phi = (rand() % 180) * M_PI / 180.0f;
            glm::vec3 particlePos = glm::vec3(r * sin(phi) * cos(theta),
                                             r * cos(phi),
                                             r * sin(phi) * sin(theta));
            cloud.particlePositions.push_back(particlePos);
            glm::vec3 velocity = glm::cross(particlePos, glm::vec3(0.0f, 1.0f, 0.0f)) * 0.1f;
            cloud.particleVelocities.push_back(velocity);
        }
        particleClouds.push_back(cloud);
    }
}

void LightScene::UpdateStations(float deltaTime) {
    for (int i = 0; i < STATION_COUNT; ++i) {
        SpaceStation& station = stations[i];
        station.currentRotation += station.rotationSpeed * deltaTime;
        station.orbitAngle += station.orbitSpeed * deltaTime;
        if (station.orbitAngle > 2.0f * M_PI) station.orbitAngle -= 2.0f * M_PI;
        station.position.x = station.orbitRadius * cos(station.orbitAngle);
        station.position.z = station.orbitRadius * sin(station.orbitAngle);
        station.turretRotation += station.turretSpeed * deltaTime;
    }
}

void LightScene::UpdateComets(float deltaTime) {
    for (auto& comet : comets) {
        comet.position += comet.velocity * deltaTime;
        comet.trailPositions.insert(comet.trailPositions.begin(), comet.position);
        if (comet.trailPositions.size() > comet.trailLength) {
            comet.trailPositions.pop_back();
        }
        comet.currentPhase += comet.pulseSpeed * deltaTime;
        comet.brightness = 0.5f + 0.5f * sin(comet.currentPhase);
        if (glm::length(comet.position) > 1200.0f) {
            float angle = (rand() % 360) * M_PI / 180.0f;
            float distance = 800.0f + (rand() % 400);
            comet.position = glm::vec3(distance * cos(angle), 
                                      ((rand() % 200) - 100) * 3.0f, 
                                      distance * sin(angle));
            glm::vec3 direction = glm::normalize(-comet.position);
            direction += glm::vec3((rand() % 100) / 200.0f - 0.25f,
                                  (rand() % 100) / 200.0f - 0.25f,
                                  (rand() % 100) / 200.0f - 0.25f);
            comet.velocity = direction * (30.0f + (rand() % 40));
            comet.trailPositions.clear();
            for (int j = 0; j < comet.trailLength; ++j) {
                comet.trailPositions.push_back(comet.position);
            }
        }
    }
}

void LightScene::UpdateDebris(float deltaTime) {
    for (auto& d : debris) {
        d.position += d.velocity * deltaTime;
        d.rotation += d.angularVelocity * deltaTime;
        d.lifetime -= deltaTime;
        if (d.lifetime <= 0.0f) {
            float radius = 100.0f + (rand() % 600);
            float theta = (rand() % 360) * M_PI / 180.0f;
            float phi = (rand() % 180) * M_PI / 180.0f;
            d.position = glm::vec3(radius * sin(phi) * cos(theta),
                                  radius * cos(phi),
                                  radius * sin(phi) * sin(theta));
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
        portal.currentRotation += portal.rotationSpeed * deltaTime;
        portal.energyFlow += portal.pulseSpeed * deltaTime;
        portal.pulseIntensity = 0.5f + 0.5f * sin(portal.energyFlow);
    }
}

void LightScene::UpdateSatellites(float deltaTime) {
    for (int i = 0; i < SATELLITE_COUNT; ++i) {
        Satellite& sat = satellites[i];
        sat.currentAngle += sat.orbitSpeed * deltaTime;
        if (sat.currentAngle > 2.0f * M_PI) sat.currentAngle -= 2.0f * M_PI;
        sat.antennaRotation.y += sat.antennaSpeed * deltaTime;
        sat.signalPulse += deltaTime * 4.0f;
        if ((rand() % 10000) < 5) {
            sat.isActive = !sat.isActive;
            sat.color = sat.isActive ? 
                       glm::vec3(0.2f, 1.0f, 0.3f) : 
                       glm::vec3(0.8f, 0.2f, 0.2f);
        }
    }
}

void LightScene::UpdateParticleClouds(float deltaTime) {
    for (auto& cloud : particleClouds) {
        cloud.currentRotation += cloud.rotationSpeed * deltaTime;
        cloud.intensity = 0.3f + 0.4f * sin(globalTime * cloud.pulseSpeed);
        for (size_t i = 0; i < cloud.particlePositions.size(); ++i) {
            cloud.particlePositions[i] += cloud.particleVelocities[i] * deltaTime;
            if (glm::length(cloud.particlePositions[i]) > cloud.radius) {
                cloud.particlePositions[i] = glm::normalize(cloud.particlePositions[i]) * cloud.radius;
                cloud.particleVelocities[i] = glm::cross(cloud.particlePositions[i], glm::vec3(0.0f, 1.0f, 0.0f)) * 0.1f;
            }
        }
    }
}

void LightScene::UpdateInteractions(float deltaTime) {
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

void LightScene::RenderStations(Camera& camera, int screenWidth, int screenHeight) {
    if (!spaceshipModel) return;
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
        glm::mat4 model = glm::translate(glm::mat4(1.0f), comet.position);
        model = glm::scale(model, glm::vec3(comet.size * comet.brightness));
        shader->setMat4("model", model);
        shader->setFloat("time", globalTime);
        lightSphere->Draw(*shader);
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
        glm::mat4 model = glm::translate(glm::mat4(1.0f), portal.position);
        model = glm::rotate(model, portal.currentRotation, glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(portal.size * (1.0f + portal.pulseIntensity * 0.3f)));
        shader->setMat4("model", model);
        shader->setFloat("time", globalTime);
        lightSphere->Draw(*shader);
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
        glm::vec3 pos = sat.basePosition + glm::vec3(
            sat.orbitRadius * cos(sat.currentAngle),
            0.0f,
            sat.orbitRadius * sin(sat.currentAngle)
        );
        glm::mat4 model = glm::translate(glm::mat4(1.0f), pos);
        model = glm::rotate(model, sat.antennaRotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(1.5f));
        shader->setMat4("model", model);
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
        glm::mat4 cloudRotation = glm::rotate(glm::mat4(1.0f), cloud.currentRotation, glm::vec3(0.0f, 1.0f, 0.0f));
        for (const auto& particlePos : cloud.particlePositions) {
            glm::vec3 worldPos = cloud.center + glm::vec3(cloudRotation * glm::vec4(particlePos, 1.0f));
            glm::mat4 model = glm::translate(glm::mat4(1.0f), worldPos);
            model = glm::scale(model, glm::vec3(0.5f * cloud.intensity));
            shader->setMat4("model", model);
            lightSphere->Draw(*shader);
        }
    }
}
