#ifndef LIGHTSCENE_H
#define LIGHTSCENE_H

#include "Scene.h"
#include "Shader.h"
#include "Sphere.h"
#include "Model.h"
#include "Skybox.h"
#include "SkyboxManager.h"
#include <memory>
#include <vector>
#include <cmath>

class Sound;
class AudioSource;

// Scène avec source de lumière et éléments dynamiques
class LightScene : public Scene {
private:
    std::unique_ptr<Sphere> lightSphere;
    std::unique_ptr<Sphere> testSphere;
    std::unique_ptr<Skybox> skybox;
    SkyboxManager::SkyboxType currentSkyboxType;

    glm::vec3 lightPosition;
    glm::vec3 lightColor;
    float lightRadius;
    float rotationSpeed;
    float currentRotation;
    bool initialized;

    // Soleil
    std::unique_ptr<Sphere> sunSphere;
    float sunRadius = 80.0f;
    glm::vec3 sunPosition = glm::vec3(0.0f);

    // Astéroïdes
    std::unique_ptr<Model> asteroidModel;
    static const int ASTEROID_COUNT = 120;
    struct AsteroidData {
        float angleOffset, radiusOffset, scale, rotationSpeed, orbitSpeed, currentAngle;
        glm::vec3 rotationAxis, color;
    };
    AsteroidData asteroids[ASTEROID_COUNT];

    // Vaisseaux
    std::unique_ptr<Model> spaceshipModel;
    static const int SPACESHIP_COUNT = 50;
    struct SpaceshipData {
        glm::vec3 color, randomOffset;
        float angleOffset, orbitRadius, orbitSpeed, currentAngle, randomPhase;
        float heightFreq1, heightFreq2, heightFreq3;
        float heightAmp1, heightAmp2, heightAmp3;
        float heightPhase1, heightPhase2, heightPhase3;
        float horizontalFreq1, horizontalFreq2;
        float horizontalAmp1, horizontalAmp2;
        float horizontalPhase1, horizontalPhase2;
        float scale;
    };
    SpaceshipData spaceships[SPACESHIP_COUNT];

    // Lune
    std::unique_ptr<Sphere> moonSphere;
    float moonRadius = 25.0f;
    float moonOrbitRadius = 400.0f;
    float moonOrbitSpeed = 0.3f;
    float moonSelfRotSpeed = 0.8f;
    float moonCurrentAngle = 0.0f;

    // Audio
    std::shared_ptr<Sound> zooSound;
    std::shared_ptr<AudioSource> ambientSource;
    std::string currentSoundName;

    // Stations spatiales
    std::unique_ptr<Model> stationModel;
    static const int STATION_COUNT = 8;
    struct SpaceStation {
        glm::vec3 position, rotationAxis, color;
        float rotationSpeed, currentRotation, scale, orbitAngle, orbitRadius, orbitSpeed;
        float turretRotation, turretSpeed;
    };
    SpaceStation stations[STATION_COUNT];

    // Comètes
    static const int COMET_COUNT = 15;
    struct Comet {
        glm::vec3 position, velocity, color;
        float trailLength, brightness, pulseSpeed, currentPhase, size;
        std::vector<glm::vec3> trailPositions;
    };
    std::vector<Comet> comets;

    // Débris
    static const int DEBRIS_COUNT = 200;
    struct SpaceDebris {
        glm::vec3 position, velocity, angularVelocity, rotation, color;
        float scale, lifetime, maxLifetime;
    };
    std::vector<SpaceDebris> debris;

    // Portails
    static const int PORTAL_COUNT = 6;
    struct EnergyPortal {
        glm::vec3 position, color1, color2;
        float rotationSpeed, currentRotation, pulseIntensity, pulseSpeed, size, energyFlow;
    };
    EnergyPortal portals[PORTAL_COUNT];

    // Satellites
    static const int SATELLITE_COUNT = 30;
    struct Satellite {
        glm::vec3 basePosition, antennaRotation, color;
        float orbitRadius, orbitSpeed, currentAngle, antennaSpeed;
        bool isActive;
        float signalPulse;
    };
    Satellite satellites[SATELLITE_COUNT];

    // Nuages de particules
    static const int PARTICLE_CLOUD_COUNT = 10;
    struct ParticleCloud {
        glm::vec3 center, color;
        float radius, rotationSpeed, currentRotation, intensity, pulseSpeed;
        std::vector<glm::vec3> particlePositions, particleVelocities;
    };
    std::vector<ParticleCloud> particleClouds;

    // Interactions
    float globalTime;
    bool attractionMode, repulsionMode;
    glm::vec3 attractionPoint;
    float attractionStrength;

    // Méthodes internes
    bool LoadShaders();
    bool CreateLightSphere();
    void RenderLight(Camera&, int, int);
    bool LoadAudio(SoundManager&);
    void ChangeSkybox(SkyboxManager::SkyboxType);
    bool LoadModels();
    void InitializeAsteroidRing();
    void InitializeSpaceships();
    void RenderAsteroidRing(Camera&, int, int);
    void RenderSpaceships(Camera&, int, int);
    void RenderMoon(Camera&, int, int);
    void RenderSun(Camera&, int, int);
    void InitializeStations();
    void InitializeComets();
    void InitializeDebris();
    void InitializePortals();
    void InitializeSatellites();
    void InitializeParticleClouds();
    void UpdateStations(float);
    void UpdateComets(float);
    void UpdateDebris(float);
    void UpdatePortals(float);
    void UpdateSatellites(float);
    void UpdateParticleClouds(float);
    void UpdateInteractions(float);
    void RenderStations(Camera&, int, int);
    void RenderComets(Camera&, int, int);
    void RenderDebris(Camera&, int, int);
    void RenderPortals(Camera&, int, int);
    void RenderSatellites(Camera&, int, int);
    void RenderParticleClouds(Camera&, int, int);

public:
    LightScene();
    virtual ~LightScene();
    bool ChangeSceneSound(const std::string&, SoundManager&);
    std::string GetCurrentSoundName() const;
    std::shared_ptr<AudioSource> GetAmbientSource() const { return ambientSource; }
    std::shared_ptr<Sound> GetAmbientSound() const { return zooSound; }
    virtual bool Initialize(Camera&, SoundManager&) override;
    virtual void Update(float, GLFWwindow*, Camera&, SoundManager&) override;
    virtual void Render(Camera&, int, int) override;
    virtual void RenderUI(GLFWwindow*, SoundManager&) override;
    virtual void Cleanup() override;
    virtual const char* GetName() const override;
    virtual void OnActivate() override;
    virtual void OnDeactivate() override;
};

#endif // LIGHTSCENE_H
