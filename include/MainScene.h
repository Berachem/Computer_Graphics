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

/**
 * @brief Scène principale contenant tous les objets 3D et effets
 * 
 * Cette scène représente la scène complète avec tous les modèles,
 * sphères, effets de lumière et sons d'ambiance.
 */
class MainScene : public Scene {
private:    // Les shaders sont maintenant gérés par le ShaderManager global

    // === Anneau d'astéroïdes ===
    std::unique_ptr<Model> asteroidModel; // Un seul modèle réutilisé
    static const int ASTEROID_COUNT = 72; // Nombre d'astéroïdes dans l'anneau (plus dense)
    
    struct AsteroidData {
        float angleOffset;      // Décalage angulaire dans l'anneau
        float radiusOffset;     // Variation du rayon orbital
        float scale;           // Échelle de l'astéroïde
        float rotationSpeed;   // Vitesse de rotation propre
        glm::vec3 rotationAxis; // Axe de rotation
        glm::vec3 color;       // Couleur de l'astéroïde
        float orbitSpeed;      // Vitesse orbitale
    };
      AsteroidData asteroids[ASTEROID_COUNT];
    
    // === Vaisseaux français ===
    std::unique_ptr<Model> spaceshipModel;
    static const int SPACESHIP_COUNT = 3;
      struct SpaceshipData {
        glm::vec3 color;        // Couleur du vaisseau (bleu, blanc, rouge)
        float angleOffset;      // Décalage angulaire entre les vaisseaux
        float orbitRadius;      // Rayon de l'orbite
        float orbitSpeed;       // Vitesse orbitale
        float currentAngle;     // Angle actuel dans l'orbite
        glm::vec3 randomOffset; // Petit décalage aléatoire pour le mouvement
        float randomPhase;      // Phase pour l'oscillation aléatoire
        
        // Paramètres pour mouvements verticaux naturels individuels
        float heightFreq1, heightFreq2, heightFreq3;    // Fréquences des oscillations verticales
        float heightAmp1, heightAmp2, heightAmp3;       // Amplitudes des oscillations verticales
        float heightPhase1, heightPhase2, heightPhase3; // Phases des oscillations verticales
        
        // Paramètres pour mouvements horizontaux individuels
        float horizontalFreq1, horizontalFreq2;         // Fréquences des oscillations horizontales
        float horizontalAmp1, horizontalAmp2;           // Amplitudes des oscillations horizontales
        float horizontalPhase1, horizontalPhase2;       // Phases des oscillations horizontales
    };
    
    SpaceshipData spaceships[SPACESHIP_COUNT];

    // === Sphères ===
    std::unique_ptr<Sphere> moonSphere;
    std::unique_ptr<Sphere> sunSphere;    // === Audio (mutualisé via SoundManager) ===
    std::shared_ptr<Sound> zooSound;
    std::shared_ptr<AudioSource> ambientSource;
    std::string currentSoundName; // Nom du son actuellement chargé
    // Skybox pour cette scène
    std::unique_ptr<Skybox> skybox;
    SkyboxManager::SkyboxType currentSkyboxType;    // === Variables de scène ===
    float sunRadius;
    bool initialized;
    
    // === Mode pilote ===
    bool pilotMode;
    int currentSpaceshipIndex; // Index du vaisseau suivi (0, 1, 2)
    glm::vec3 lastSpaceshipPosition; // Position précédente pour calculer le vecteur de déplacement// === Méthodes privées ===
    bool LoadShaders();
    bool LoadModels();    bool LoadAudio(SoundManager& soundManager);    void InitializeAsteroidRing();
    void InitializeSpaceships();
    void RenderObjects(Camera& camera, int screenWidth, int screenHeight);
    void ChangeSkybox(SkyboxManager::SkyboxType newType);
      // === Méthodes pour le mode pilote ===
    void TogglePilotMode();
    void UpdatePilotCamera(Camera& camera);
    glm::vec3 GetSpaceshipPosition(int index) const;

public:
    /**
     * @brief Constructeur
     */
    MainScene();    // === Méthodes publiques pour le mode pilote ===
    bool IsPilotMode() const { return pilotMode; }
    
    // === Méthode pour changer le son de la scène ===
    bool ChangeSceneSound(const std::string& soundName, SoundManager& soundManager);
    
    // === Méthode pour obtenir le nom du son actuel ===
    std::string GetCurrentSoundName() const;

    /**
     * @brief Destructeur
     */
    virtual ~MainScene();

    // === Méthodes héritées de Scene ===
    virtual bool Initialize(Camera& camera, SoundManager& soundManager) override;
    virtual void Update(float deltaTime, GLFWwindow* window, Camera& camera, SoundManager& soundManager) override;
    virtual void Render(Camera& camera, int screenWidth, int screenHeight) override;
    virtual void RenderUI(GLFWwindow* window, SoundManager& soundManager) override;
    virtual void Cleanup() override;    virtual const char* GetName() const override;
    virtual void OnActivate() override;
    virtual void OnDeactivate() override;
    
    // === Méthode pour positionner la caméra ===
    void SetupCameraOverview(Camera& camera);
};

#endif // MAINSCENE_H
