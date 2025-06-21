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
    bool initialized;    // === Audio (mutualisé via SoundManager) ===
    std::shared_ptr<Sound> zooSound;
    std::shared_ptr<AudioSource> ambientSource;
    std::string currentSoundName; // Nom du son actuellement chargé

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
