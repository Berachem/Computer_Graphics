#include "UIHelpers.h"
#include "SkyboxManager.h"
#include "ShaderManager.h"
#include <vector>
#include <string>
#include "SoundManager.h"
#include "AudioSource.h"
#include "Sound.h"
#include "imgui.h"
#include <glm/glm.hpp>

namespace UIHelpers {

void RenderAudioUI(GLFWwindow* window, SoundManager& soundManager, std::shared_ptr<AudioSource> source, std::shared_ptr<Sound> sound, const std::string& currentSoundName, std::function<bool(const std::string&)> changeSoundCallback) {
    if (!soundManager.IsInitialized()) return;
    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowCollapsed(true, ImGuiCond_FirstUseEver);
    ImGui::Begin("Contrôles Audio", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
    float masterVolume = soundManager.GetMasterVolume();
    ImGui::Text("Volume Principal: %.0f%%", masterVolume * 100.0f);
    if (ImGui::SliderFloat("##MasterVolume", &masterVolume, 0.0f, 1.0f, "%.2f")) {
        soundManager.SetMasterVolume(masterVolume);
    }
    ImGui::Separator();
    ImGui::Text("Sélection du son:");
    std::vector<std::string> soundNames = soundManager.GetSoundNames();
    if (!soundNames.empty()) {
        int currentIndex = 0;
        for (size_t i = 0; i < soundNames.size(); ++i) {
            if (soundNames[i] == currentSoundName) { currentIndex = static_cast<int>(i); break; }
        }
        if (ImGui::Combo("Son", &currentIndex, [](void* data, int idx, const char** out_text) {
            std::vector<std::string>* names = static_cast<std::vector<std::string>*>(data);
            if (idx >= 0 && idx < static_cast<int>(names->size())) { *out_text = (*names)[idx].c_str(); return true; }
            return false; }, &soundNames, static_cast<int>(soundNames.size()))) {
            if (currentIndex >= 0 && currentIndex < static_cast<int>(soundNames.size()) && changeSoundCallback) {
                changeSoundCallback(soundNames[currentIndex]);
            }
        }
    }
    std::shared_ptr<Sound> currentSound = sound;
    if (currentSound && source) {
        ImGui::Text("Son d'ambiance: %s", currentSound->GetFileName().c_str());
        ImGui::Text("Durée: %.1fs", currentSound->GetDuration());
        bool isPlaying = source->IsPlaying();
        bool isPaused = source->IsPaused();
        if (isPlaying && !isPaused) ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "État: En cours");
        else if (isPaused) ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "État: En pause");
        else ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "État: Arrêté");
        if (!isPlaying || isPaused) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.7f, 0.0f, 1.0f));
            if (ImGui::Button(isPaused ? "Reprendre" : "Jouer")) source->Play(currentSound, true);
            ImGui::PopStyleColor();
        }
        if (isPlaying && !isPaused) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7f, 0.7f, 0.0f, 1.0f));
            if (ImGui::Button("Pause")) source->Pause();
            ImGui::PopStyleColor();
        }
        if (isPlaying) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7f, 0.0f, 0.0f, 1.0f));
            if (ImGui::Button("Arrêter")) source->Stop();
            ImGui::PopStyleColor();
        }
    }
    ImGui::End();
}

void RenderMainControlsUI(SkyboxManager::SkyboxType& currentSkyboxType, std::function<void(SkyboxManager::SkyboxType)> onSkyboxChange) {
    ImGui::Begin("Contrôles Principaux", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
    int skyboxIndex = static_cast<int>(currentSkyboxType);
    const char* skyboxNames[] = { "Colorée", "Défaut", "Espace", "Zoo" };
    if (ImGui::Combo("Skybox", &skyboxIndex, skyboxNames, IM_ARRAYSIZE(skyboxNames))) {
        currentSkyboxType = static_cast<SkyboxManager::SkyboxType>(skyboxIndex);
        if (onSkyboxChange) onSkyboxChange(currentSkyboxType);
    }
    ImGui::End();
}

void RenderSkyboxUI(const char* title, SkyboxManager::SkyboxType& currentType, std::function<void(SkyboxManager::SkyboxType)> onChange) {
    ImGui::Begin(title, nullptr, ImGuiWindowFlags_AlwaysAutoResize);
    int skyboxIndex = static_cast<int>(currentType);
    const char* skyboxNames[] = { "Colorée", "Défaut", "Espace", "Zoo" };
    if (ImGui::Combo("Skybox", &skyboxIndex, skyboxNames, IM_ARRAYSIZE(skyboxNames))) {
        currentType = static_cast<SkyboxManager::SkyboxType>(skyboxIndex);
        if (onChange) onChange(currentType);
    }
    ImGui::End();
}

void RenderKeyboardUI(GLFWwindow* window) {
    ImGui::Begin("Contrôles Clavier", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::Text("ZQSD : Déplacement");
    ImGui::Text("Espace : Monter");
    ImGui::Text("Ctrl : Descendre");
    ImGui::Text("Souris : Orientation");
    ImGui::End();
}

} // namespace UIHelpers
