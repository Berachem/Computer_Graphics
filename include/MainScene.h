#ifndef MAINSCENE_H
#define MAINSCENE_H

#include "Scene.h"
#include "Shader.h"
#include "Model.h"
#include "Sphere.h"
#include "Camera.h"
#include "AudioSource.h"
#include "Sound.h"
#include "UIHelpers.h"
#include "Skybox.h"
#include "SkyboxManager.h"
#include <memory>
#include <glm/glm.hpp>

// Scène principale avec objets 3D et effets
class MainScene : public Scene {
private:
    std::unique_ptr<Model> asteroidModel;
    static const int ASTEROID_COUNT = 72;
    struct AsteroidData {
        float angleOffset, radiusOffset, scale, rotationSpeed, orbitSpeed;
        glm::vec3 rotationAxis, color;
    };
    AsteroidData asteroids[ASTEROID_COUNT];

    std::unique_ptr<Model> spaceshipModel;
    static const int SPACESHIP_COUNT = 3;
    struct SpaceshipData {
        glm::vec3 color, randomOffset;
        float angleOffset, orbitRadius, orbitSpeed, currentAngle, randomPhase;
        float heightFreq1, heightFreq2, heightFreq3;
        float heightAmp1, heightAmp2, heightAmp3;
        float heightPhase1, heightPhase2, heightPhase3;
        float horizontalFreq1, horizontalFreq2;
        float horizontalAmp1, horizontalAmp2;
        float horizontalPhase1, horizontalPhase2;
    };
    SpaceshipData spaceships[SPACESHIP_COUNT];

    std::unique_ptr<Sphere> moonSphere;
    std::unique_ptr<Sphere> sunSphere;
    std::shared_ptr<Sound> zooSound;
    std::shared_ptr<AudioSource> ambientSource;
    std::string currentSoundName;
    std::unique_ptr<Skybox> skybox;
    SkyboxManager::SkyboxType currentSkyboxType;
    float sunRadius;
    bool initialized;
    bool pilotMode;
    int currentSpaceshipIndex;
    glm::vec3 lastSpaceshipPosition;

    // Méthodes internes
    bool LoadShaders();
    bool LoadModels();
    bool LoadAudio(SoundManager&);
    void InitializeAsteroidRing();
    void InitializeSpaceships();
    void RenderObjects(Camera&, int, int);
    void ChangeSkybox(SkyboxManager::SkyboxType);
    void TogglePilotMode();
    void UpdatePilotCamera(Camera&);
    glm::vec3 GetSpaceshipPosition(int) const;

public:
    MainScene();
    bool IsPilotMode() const { return pilotMode; }
    bool ChangeSceneSound(const std::string&, SoundManager&);
    std::string GetCurrentSoundName() const;
    std::shared_ptr<AudioSource> GetAmbientSource() const { return ambientSource; }
    std::shared_ptr<Sound> GetAmbientSound() const { return zooSound; }
    virtual ~MainScene();
    virtual bool Initialize(Camera&, SoundManager&) override;
    virtual void Update(float, GLFWwindow*, Camera&, SoundManager&) override;
    virtual void Render(Camera&, int, int) override;
    virtual void RenderUI(GLFWwindow*, SoundManager&) override;
    virtual void Cleanup() override;
    virtual const char* GetName() const override;
    virtual void OnActivate() override;
    virtual void OnDeactivate() override;
    void SetupCameraOverview(Camera& camera);
};

#endif // MAINSCENE_H
