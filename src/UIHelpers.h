#ifndef UIHELPERS_H
#define UIHELPERS_H

#include <GLFW/glfw3.h>
#include "SoundManager.h"
#include <memory>

class Sound;
class AudioSource;

namespace UIHelpers {
    void RenderAudioUI(GLFWwindow* window, SoundManager& soundManager, std::shared_ptr<AudioSource> source = nullptr, std::shared_ptr<Sound> sound = nullptr);
    void RenderKeyboardUI(GLFWwindow* window);
}

#endif // UIHELPERS_H
