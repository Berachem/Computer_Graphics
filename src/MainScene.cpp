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

static float luneOrbitRadius = 250.0f;
static float luneOrbitSpeed = 0.50f;
static float luneSelfRotSpeed = 0.5f;
static float sunDistance = 100.0f;
static glm::vec3 lightPosition = glm::vec3(-100.0f, 15.0f, -100.0f);

MainScene::MainScene()
    : sunRadius(45.0f), initialized(false), pilotMode(false), currentSpaceshipIndex(0), lastSpaceshipPosition(0.0f) {}

MainScene::~MainScene() { Cleanup(); }

bool MainScene::Initialize(Camera& camera, SoundManager& soundManager) {
    if (initialized) return true;
    if (!LoadShaders()) return false;
    if (!LoadModels()) return false;
    LoadAudio(soundManager);
    currentSkyboxType = SkyboxManager::SkyboxType::SPACE;
    skybox = std::make_unique<Skybox>(SkyboxManager::GetSkyboxFaces(currentSkyboxType));
    SetupCameraOverview(camera);
    initialized = true;
    return true;
}

bool MainScene::LoadShaders() { return true; }

bool MainScene::LoadModels() {
    try {
        asteroidModel = std::make_unique<Model>("../models/astroid.obj");
        spaceshipModel = std::make_unique<Model>("../models/map-bump.obj");
        InitializeAsteroidRing();
        InitializeSpaceships();
        moonSphere = std::make_unique<Sphere>("../textures/spherical_moon_texture.jpg", 9.0f, 36, 18);
        sunSphere = std::make_unique<Sphere>("", sunRadius, 36, 18);
        return true;
    } catch (...) { return false; }
}

bool MainScene::LoadAudio(SoundManager& soundManager) {
    if (!soundManager.IsInitialized()) return false;
    std::string soundName = "spatial_theme";
    zooSound = soundManager.GetSound(soundName);
    if (!zooSound) {
        soundName = "Zoo";
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

void MainScene::Update(float deltaTime, GLFWwindow* window, Camera& camera, SoundManager& soundManager) {
    if (!initialized) return;
    static bool vKeyPressed = false;
    bool vKeyDown = (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS);
    if (vKeyDown && !vKeyPressed) {
        TogglePilotMode();
    }
    vKeyPressed = vKeyDown;
    if (soundManager.IsInitialized()) {
        float listenerPos[3] = {camera.Position.x, camera.Position.y, camera.Position.z};
        float listenerForward[3] = {camera.Front.x, camera.Front.y, camera.Front.z};
        float listenerUp[3] = {camera.Up.x, camera.Up.y, camera.Up.z};
        soundManager.SetListenerPosition(listenerPos, listenerForward, listenerUp);
    }
    float currentFrame = static_cast<float>(glfwGetTime());
    float angle = currentFrame * 0.0005f;
    lightPosition.x = -sunDistance * cos(angle);
    lightPosition.z = -sunDistance * sin(angle);
    lightPosition.y = 15.0f;
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
}

// ...existing code...
