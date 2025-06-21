#ifndef UIHELPERS_H
#define UIHELPERS_H

#include <GLFW/glfw3.h>
#include "SoundManager.h"
#include "SkyboxManager.h"
#include <memory>
#include <functional>

// Forward declarations
class Sound;
class AudioSource;
class ShaderManager;
enum class LightingShaderType;

namespace UIHelpers {
    /**
     * @brief Interface audio mutualisée
     */
    void RenderAudioUI(GLFWwindow* window, SoundManager& soundManager,
                       std::shared_ptr<AudioSource> source = nullptr,
                       std::shared_ptr<Sound> sound = nullptr);

    /**
     * @brief Panneau de contrôles principaux unifié (shader + skybox)
     * @param currentSkyboxType Référence au type de skybox courant
     * @param onSkyboxChange Callback appelé lors du changement de skybox
     */
    void RenderMainControlsUI(SkyboxManager::SkyboxType& currentSkyboxType, 
                              std::function<void(SkyboxManager::SkyboxType)> onSkyboxChange);

    /**
     * @brief Interface de sélection d'une skybox via un menu déroulant (obsolète, utiliser RenderMainControlsUI)
     * @param title Titre de la fenêtre ImGui
     * @param currentType Référence au type de skybox courant
     * @param onChange Callback appelé lors du changement de sélection
     */
    void RenderSkyboxUI(const char* title, SkyboxManager::SkyboxType& currentType, std::function<void(SkyboxManager::SkyboxType)> onChange);
}

#endif // UIHELPERS_H
