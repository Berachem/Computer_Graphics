#ifndef MAINSCENE_H
#define MAINSCENE_H

#include "Scene.h"
#include "Shader.h"
#include "Model.h"
#include "Sphere.h"
#include "AudioSource.h"
#include "Sound.h"
#include "UIHelpers.h"
#include "Skybox.h"
#include "SkyboxManager.h"
#include <memory>
#include <glm/glm.hpp>

/**
 * @brief Scène principale contenant tous les objets 3D et effets
 * 
 * Cette scène représente la scène complète avec tous les modèles,
 * sphères, effets de lumière et sons d'ambiance.
 */
class MainScene : public Scene {
private:
    // Les shaders sont maintenant gérés par le ShaderManager global

    // === Modèles 3D ===
    std::unique_ptr<Model> myModel;
    std::unique_ptr<Model> asteroid1;
    std::unique_ptr<Model> asteroid2;
    std::unique_ptr<Model> asteroid3;
    std::unique_ptr<Model> asteroid4;    // === Sphères ===
    std::unique_ptr<Sphere> moonSphere;
    std::unique_ptr<Sphere> sunSphere;
    std::unique_ptr<Sphere> testSphere; // Sphère de test pour comparer les shaders

    // === Audio (mutualisé via SoundManager) ===
    std::shared_ptr<Sound> zooSound;
    std::shared_ptr<AudioSource> ambientSource;
    // Skybox pour cette scène
    std::unique_ptr<Skybox> skybox;
    SkyboxManager::SkyboxType currentSkyboxType;

    // === Variables de scène ===
    float sunRadius;
    bool initialized;

    // === Méthodes privées ===
    bool LoadShaders();
    bool LoadModels();
    bool LoadAudio(SoundManager& soundManager);
    void RenderObjects(Camera& camera, int screenWidth, int screenHeight);
    void ChangeSkybox(SkyboxManager::SkyboxType newType);

public:
    /**
     * @brief Constructeur
     */
    MainScene();

    /**
     * @brief Destructeur
     */
    virtual ~MainScene();

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

#endif // MAINSCENE_H
