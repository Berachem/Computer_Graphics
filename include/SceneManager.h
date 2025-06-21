#ifndef SCENEMANAGER_H
#define SCENEMANAGER_H

#include <memory>
#include <vector>
#include <string>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "Scene.h"
#include "Camera.h"
#include "SoundManager.h"

/**
 * @brief Gestionnaire de scènes pour l'application
 * 
 * Cette classe gère le cycle de vie des différentes scènes de l'application,
 * permettant de basculer entre elles et de les mettre à jour.
 */
class SceneManager {
private:
    std::vector<std::unique_ptr<Scene>> scenes;  ///< Liste des scènes disponibles
    int currentSceneIndex;                       ///< Index de la scène actuelle
    bool initialized;                            ///< État d'initialisation

public:
    /**
     * @brief Constructeur par défaut
     */
    SceneManager();

    /**
     * @brief Destructeur
     */
    ~SceneManager();

    /**
     * @brief Initialise le gestionnaire de scènes
     * @param camera Référence vers la caméra principale
     * @param soundManager Référence vers le gestionnaire audio
     * @return true si l'initialisation réussit, false sinon
     */
    bool Initialize(Camera& camera, SoundManager& soundManager);

    /**
     * @brief Ajoute une nouvelle scène au gestionnaire
     * @param scene Pointeur unique vers la scène à ajouter
     */
    void AddScene(std::unique_ptr<Scene> scene);

    /**
     * @brief Change vers la scène suivante
     */
    void NextScene();

    /**
     * @brief Change vers la scène précédente
     */
    void PreviousScene();

    /**
     * @brief Change vers une scène spécifique par index
     * @param index Index de la scène cible
     * @return true si le changement réussit, false sinon
     */
    bool SetCurrentScene(int index);

    /**
     * @brief Met à jour la scène actuelle
     * @param deltaTime Temps écoulé depuis la dernière frame
     * @param window Pointeur vers la fenêtre GLFW
     * @param camera Référence vers la caméra
     * @param soundManager Référence vers le gestionnaire audio
     */
    void Update(float deltaTime, GLFWwindow* window, Camera& camera, SoundManager& soundManager);

    /**
     * @brief Effectue le rendu de la scène actuelle
     * @param camera Référence vers la caméra
     * @param screenWidth Largeur de l'écran
     * @param screenHeight Hauteur de l'écran
     */
    void Render(Camera& camera, int screenWidth, int screenHeight);

    /**
     * @brief Effectue le rendu de l'interface utilisateur de la scène actuelle
     * @param window Pointeur vers la fenêtre GLFW
     * @param soundManager Référence vers le gestionnaire audio
     */
    void RenderUI(GLFWwindow* window, SoundManager& soundManager);

    /**
     * @brief Retourne le nombre de scènes disponibles
     * @return Nombre de scènes
     */
    int GetSceneCount() const;

    /**
     * @brief Retourne l'index de la scène actuelle
     * @return Index de la scène actuelle
     */
    int GetCurrentSceneIndex() const;

    /**
     * @brief Retourne le nom de la scène actuelle
     * @return Nom de la scène actuelle, ou "Aucune" si pas de scène
     */
    const char* GetCurrentSceneName() const;

    /**
     * @brief Retourne un pointeur vers la scène actuelle
     * @return Pointeur vers la scène actuelle, nullptr si pas de scène
     */
    Scene* GetCurrentScene() const;

    /**
     * @brief Vérifie si le gestionnaire est initialisé
     * @return true si initialisé, false sinon
     */
    bool IsInitialized() const;

    /**
     * @brief Nettoie toutes les ressources
     */
    void Cleanup();
};

#endif // SCENEMANAGER_H
