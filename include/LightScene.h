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

// Forward declarations
class Sound;
class AudioSource;

/**
 * @brief Scène simple avec seulement une source de lumière
 * 
 * Cette scène contient uniquement une sphère représentant une source
 * de lumière, permettant de tester les effets d'éclairage de base.
 */
class LightScene : public Scene {
private:
    // Les shaders sont maintenant gérés par le ShaderManager global    // === Objets 3D ===
    std::unique_ptr<Sphere> lightSphere;
    std::unique_ptr<Sphere> testSphere; // Sphère de test pour comparer les shaders
    std::unique_ptr<Skybox> skybox;
    SkyboxManager::SkyboxType currentSkyboxType;

    // === Variables de scène ===
    glm::vec3 lightPosition;
    glm::vec3 lightColor;
    float lightRadius;
    float rotationSpeed;
    float currentRotation;
    bool initialized;

    // === NOUVEAU CONTENU SPECTACULAIRE ===
    
    // Soleil central imposant
    std::unique_ptr<Sphere> sunSphere;
    float sunRadius = 80.0f;
    glm::vec3 sunPosition = glm::vec3(0.0f, 0.0f, 0.0f);
    
    // Anneau d'astéroïdes dense
    std::unique_ptr<Model> asteroidModel;
    static const int ASTEROID_COUNT = 120;
    struct AsteroidData {
        float angleOffset;
        float radiusOffset;
        float scale;
        float rotationSpeed;
        glm::vec3 rotationAxis;
        glm::vec3 color;
        float orbitSpeed;
        float currentAngle;
    };
    AsteroidData asteroids[ASTEROID_COUNT];
    
    // Flotte de vaisseaux spatiaux
    std::unique_ptr<Model> spaceshipModel;
    static const int SPACESHIP_COUNT = 50;
    struct SpaceshipData {
        glm::vec3 color;
        float angleOffset;
        float orbitRadius;
        float orbitSpeed;
        float currentAngle;
        glm::vec3 randomOffset;
        float randomPhase;
        float heightFreq1, heightFreq2, heightFreq3;
        float heightAmp1, heightAmp2, heightAmp3;
        float heightPhase1, heightPhase2, heightPhase3;
        float horizontalFreq1, horizontalFreq2;
        float horizontalAmp1, horizontalAmp2;
        float horizontalPhase1, horizontalPhase2;
        float scale;
    };
    SpaceshipData spaceships[SPACESHIP_COUNT];
    
    // Lune en orbite lointaine
    std::unique_ptr<Sphere> moonSphere;
    float moonRadius = 25.0f;
    float moonOrbitRadius = 400.0f;
    float moonOrbitSpeed = 0.3f;
    float moonSelfRotSpeed = 0.8f;
    float moonCurrentAngle = 0.0f;

    // === Audio (mutualisé via SoundManager) ===
    std::shared_ptr<Sound> zooSound;
    std::shared_ptr<AudioSource> ambientSource;
    std::string currentSoundName; // Nom du son actuellement chargé

    // === NOUVEAUX ÉLÉMENTS DYNAMIQUES ===
    
    // Stations spatiales rotatives
    std::unique_ptr<Model> stationModel;
    static const int STATION_COUNT = 8;
    struct SpaceStation {
        glm::vec3 position;
        float rotationSpeed;
        float currentRotation;
        glm::vec3 rotationAxis;
        float scale;
        glm::vec3 color;
        float orbitAngle;
        float orbitRadius;
        float orbitSpeed;
        // Défenses tournantes
        float turretRotation;
        float turretSpeed;
    };
    SpaceStation stations[STATION_COUNT];
    
    // Comètes avec traînées
    static const int COMET_COUNT = 15;
    struct Comet {
        glm::vec3 position;
        glm::vec3 velocity;
        float trailLength;
        std::vector<glm::vec3> trailPositions;
        glm::vec3 color;
        float brightness;
        float pulseSpeed;
        float currentPhase;
        float size;
    };
    std::vector<Comet> comets;
    
    // Débris spatiaux en mouvement chaotique
    static const int DEBRIS_COUNT = 200;
    struct SpaceDebris {
        glm::vec3 position;
        glm::vec3 velocity;
        glm::vec3 angularVelocity;
        glm::vec3 rotation;
        float scale;
        glm::vec3 color;
        float lifetime;
        float maxLifetime;
    };
    std::vector<SpaceDebris> debris;
    
    // Portails énergétiques rotatifs
    static const int PORTAL_COUNT = 6;
    struct EnergyPortal {
        glm::vec3 position;
        float rotationSpeed;
        float currentRotation;
        float pulseIntensity;
        float pulseSpeed;
        glm::vec3 color1;
        glm::vec3 color2;
        float size;
        float energyFlow;
    };
    EnergyPortal portals[PORTAL_COUNT];
    
    // Satellites en formation
    static const int SATELLITE_COUNT = 30;
    struct Satellite {
        glm::vec3 basePosition;
        float orbitRadius;
        float orbitSpeed;
        float currentAngle;
        glm::vec3 antennaRotation;
        float antennaSpeed;
        glm::vec3 color;
        bool isActive;
        float signalPulse;
    };
    Satellite satellites[SATELLITE_COUNT];
    
    // Nuages de particules énergétiques
    static const int PARTICLE_CLOUD_COUNT = 10;
    struct ParticleCloud {
        glm::vec3 center;
        float radius;
        float rotationSpeed;
        float currentRotation;
        glm::vec3 color;
        float intensity;
        float pulseSpeed;
        std::vector<glm::vec3> particlePositions;
        std::vector<glm::vec3> particleVelocities;
    };
    std::vector<ParticleCloud> particleClouds;
    
    // Interactions dynamiques
    float globalTime;
    bool attractionMode;
    bool repulsionMode;
    glm::vec3 attractionPoint;
    float attractionStrength;
    
    // === Méthodes privées ===
    bool LoadShaders();
    bool CreateLightSphere();
    void RenderLight(Camera& camera, int screenWidth, int screenHeight);
    bool LoadAudio(SoundManager& soundManager);
    void ChangeSkybox(SkyboxManager::SkyboxType newType);
    
    // Nouvelles méthodes pour le contenu spectaculaire
    bool LoadModels();
    void InitializeAsteroidRing();
    void InitializeSpaceships();
    void RenderAsteroidRing(Camera& camera, int screenWidth, int screenHeight);
    void RenderSpaceships(Camera& camera, int screenWidth, int screenHeight);
    void RenderMoon(Camera& camera, int screenWidth, int screenHeight);
    void RenderSun(Camera& camera, int screenWidth, int screenHeight);

    // Nouvelles méthodes pour les éléments dynamiques
    void InitializeStations();
    void InitializeComets();
    void InitializeDebris();
    void InitializePortals();
    void InitializeSatellites();
    void InitializeParticleClouds();
    
    void UpdateStations(float deltaTime);
    void UpdateComets(float deltaTime);
    void UpdateDebris(float deltaTime);
    void UpdatePortals(float deltaTime);
    void UpdateSatellites(float deltaTime);
    void UpdateParticleClouds(float deltaTime);
    void UpdateInteractions(float deltaTime);
    
    void RenderStations(Camera& camera, int screenWidth, int screenHeight);
    void RenderComets(Camera& camera, int screenWidth, int screenHeight);
    void RenderDebris(Camera& camera, int screenWidth, int screenHeight);
    void RenderPortals(Camera& camera, int screenWidth, int screenHeight);
    void RenderSatellites(Camera& camera, int screenWidth, int screenHeight);
    void RenderParticleClouds(Camera& camera, int screenWidth, int screenHeight);
    
public:
    /**
     * @brief Constructeur
     */
    LightScene();    /**
     * @brief Destructeur
     */
    virtual ~LightScene();    // === Méthode pour changer le son de la scène ===
    bool ChangeSceneSound(const std::string& soundName, SoundManager& soundManager);
      // === Méthode pour obtenir le nom du son actuel ===
    std::string GetCurrentSoundName() const;
    
    // === Méthodes pour accéder à l'audio de la scène ===
    std::shared_ptr<AudioSource> GetAmbientSource() const { return ambientSource; }
    std::shared_ptr<Sound> GetAmbientSound() const { return zooSound; }

    // === Méthodes héritées de Scene ===
    virtual bool Initialize(Camera& camera, SoundManager& soundManager) override;
    virtual void Update(float deltaTime, GLFWwindow* window, Camera& camera, SoundManager& soundManager) override;
    virtual void Render(Camera& camera, int screenWidth, int screenHeight) override;
    virtual void RenderUI(GLFWwindow* window, SoundManager& soundManager) override;
    virtual void Cleanup() override;
    virtual const char* GetName() const override;
    virtual void OnActivate() override;
    virtual void OnDeactivate() override;
};

#endif // LIGHTSCENE_H
