#ifndef LIGHTSCENE_H
#define LIGHTSCENE_H

#include "Scene.h"
#include "Shader.h"
#include "Sphere.h"
#include "Skybox.h"
#include "SkyboxManager.h"
#include <memory>
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
    // === Shaders ===
    std::unique_ptr<Shader> lightShader;

    // === Objets 3D ===
    std::unique_ptr<Sphere> lightSphere;
    std::unique_ptr<Skybox> skybox;
    SkyboxManager::SkyboxType currentSkyboxType;

    // === Variables de scène ===
    glm::vec3 lightPosition;
    glm::vec3 lightColor;
    float lightRadius;
    float rotationSpeed;
    float currentRotation;
    bool initialized;

    // === Audio (mutualisé via SoundManager) ===
    std::shared_ptr<Sound> zooSound;
    std::shared_ptr<AudioSource> ambientSource;

    // === Méthodes privées ===
    bool LoadShaders();
    bool CreateLightSphere();
    void RenderLight(Camera& camera, int screenWidth, int screenHeight);
    bool LoadAudio(SoundManager& soundManager);
    void ChangeSkybox(SkyboxManager::SkyboxType newType);

public:
    /**
     * @brief Constructeur
     */
    LightScene();

    /**
     * @brief Destructeur
     */
    virtual ~LightScene();

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
