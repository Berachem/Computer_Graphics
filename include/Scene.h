#ifndef SCENE_H
#define SCENE_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include "Camera.h"
#include "SoundManager.h"

/**
 * @brief Classe de base abstraite pour toutes les scènes
 * 
 * Cette classe définit l'interface commune que toutes les scènes doivent implémenter.
 * Elle gère les opérations de base comme l'initialisation, la mise à jour et le rendu.
 */
class Scene {
public:
    /**
     * @brief Constructeur virtuel par défaut
     */
    virtual ~Scene() = default;

    /**
     * @brief Initialise la scène
     * @param camera Référence vers la caméra principale
     * @param soundManager Référence vers le gestionnaire audio
     * @return true si l'initialisation réussit, false sinon
     */
    virtual bool Initialize(Camera& camera, SoundManager& soundManager) = 0;

    /**
     * @brief Met à jour la logique de la scène
     * @param deltaTime Temps écoulé depuis la dernière frame
     * @param window Pointeur vers la fenêtre GLFW
     * @param camera Référence vers la caméra
     * @param soundManager Référence vers le gestionnaire audio
     */
    virtual void Update(float deltaTime, GLFWwindow* window, Camera& camera, SoundManager& soundManager) = 0;

    /**
     * @brief Effectue le rendu de la scène
     * @param camera Référence vers la caméra
     * @param screenWidth Largeur de l'écran
     * @param screenHeight Hauteur de l'écran
     */
    virtual void Render(Camera& camera, int screenWidth, int screenHeight) = 0;

    /**
     * @brief Effectue le rendu de l'interface utilisateur de la scène
     * @param window Pointeur vers la fenêtre GLFW
     * @param soundManager Référence vers le gestionnaire audio
     */
    virtual void RenderUI(GLFWwindow* window, SoundManager& soundManager) = 0;

    /**
     * @brief Nettoie les ressources de la scène
     */
    virtual void Cleanup() = 0;

    /**
     * @brief Retourne le nom de la scène
     * @return Nom de la scène
     */
    virtual const char* GetName() const = 0;

    /**
     * @brief Appelée when la scène devient active
     */
    virtual void OnActivate() {}

    /**
     * @brief Appelée when la scène devient inactive
     */
    virtual void OnDeactivate() {}

    /**
     * @brief Change le son de la scène
     * @param soundName Nom du son à charger
     * @param soundManager Référence vers le gestionnaire audio
     * @return true si le changement réussit, false sinon
     */
    virtual bool ChangeSceneSound(const std::string& soundName, SoundManager& soundManager) { return false; }
};

#endif // SCENE_H
