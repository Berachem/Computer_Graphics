#ifndef UIHELPERS_H
#define UIHELPERS_H

#include <GLFW/glfw3.h>
#include "SoundManager.h"
#include "SkyboxManager.h"
#include <memory>
#include <functional>

class Sound;
class AudioSource;

namespace UIHelpers {
    /**
     * @brief Interface audio mutualisée
     */
    void RenderAudioUI(GLFWwindow* window, SoundManager& soundManager,
                       std::shared_ptr<AudioSource> source = nullptr,
                       std::shared_ptr<Sound> sound = nullptr);

    /**
     * @brief Clavier virtuel mutalisé
     */
    void RenderKeyboardUI(GLFWwindow* window);

    /**
     * @brief Interface de sélection d'une skybox via un menu déroulant
     * @param title Titre de la fenêtre ImGui
     * @param currentType Référence au type de skybox courant
     * @param onChange Callback appelé lors du changement de sélection
     */
    void RenderSkyboxUI(const char* title, SkyboxManager::SkyboxType& currentType, std::function<void(SkyboxManager::SkyboxType)> onChange);
}

#endif // UIHELPERS_H
