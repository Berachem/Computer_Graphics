#include "UIHelpers.h"
// include pour gestion des skyboxes et types
#include "SkyboxManager.h"
#include "ShaderManager.h"
#include <vector>
#include <string>
#include <iostream>

#include "SoundManager.h"
#include "AudioSource.h"
#include "Sound.h"
#include "imgui.h"
#include <glm/glm.hpp>

namespace UIHelpers {

void RenderAudioUI(GLFWwindow* window, SoundManager& soundManager, std::shared_ptr<AudioSource> source, std::shared_ptr<Sound> sound) {
    if (!soundManager.IsInitialized()) return;

    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
    ImGui::Begin("Contrôles Audio", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

    float masterVolume = soundManager.GetMasterVolume();
    ImGui::Text("Volume Principal: %.0f%%", masterVolume * 100.0f);
    if (ImGui::SliderFloat("##MasterVolume", &masterVolume, 0.0f, 1.0f, "%.2f")) {
        soundManager.SetMasterVolume(masterVolume);
    }

    ImGui::Separator();

    // Sélection du son
    ImGui::Text("Sélection du son:");
    std::vector<std::string> soundNames = soundManager.GetSoundNames();
    std::string currentSoundName = soundManager.GetCurrentSoundName();
    
    if (!soundNames.empty()) {
        // Trouver l'index du son actuel
        int currentIndex = 0;
        for (size_t i = 0; i < soundNames.size(); ++i) {
            if (soundNames[i] == currentSoundName) {
                currentIndex = static_cast<int>(i);
                break;
            }
        }
          // Menu déroulant pour sélectionner le son
        if (ImGui::Combo("Son", &currentIndex, [](void* data, int idx, const char** out_text) {
            std::vector<std::string>* names = static_cast<std::vector<std::string>*>(data);
            if (idx >= 0 && idx < static_cast<int>(names->size())) {
                *out_text = (*names)[idx].c_str();
                return true;
            }
            return false;
        }, &soundNames, static_cast<int>(soundNames.size()))) {
            // Changement de son
            if (currentIndex >= 0 && currentIndex < static_cast<int>(soundNames.size())) {
                soundManager.SetCurrentAmbientSound(soundNames[currentIndex]);
                
                // Lancer automatiquement le nouveau son
                if (source) {
                    std::shared_ptr<Sound> newSound = soundManager.GetAmbientSound();
                    if (newSound) {
                        source->Play(newSound, true);
                        std::cout << "Nouveau son lancé automatiquement: " << newSound->GetFileName() << std::endl;
                    }
                }
            }
        }
    }    ImGui::Separator();

    // Récupérer le son actuel depuis le SoundManager
    std::shared_ptr<Sound> currentSound = soundManager.GetAmbientSound();
    
    if (currentSound && source) {
        ImGui::Text("Son d'ambiance: %s", currentSound->GetFileName().c_str());
        ImGui::Text("Durée: %.1fs", currentSound->GetDuration());

        bool isPlaying = source->IsPlaying();
        bool isPaused = source->IsPaused();

        // Debug des états
        ImGui::Text("Debug - Playing: %s, Paused: %s", isPlaying ? "true" : "false", isPaused ? "true" : "false");

        if (isPlaying && !isPaused) {
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "État: En cours");
        } else if (isPaused) {
            ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "État: En pause");
        } else {
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "État: Arrêté");
        }        // Boutons de contrôle - toujours utiliser currentSound
        if (!isPlaying || isPaused) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.7f, 0.0f, 1.0f));
            if (ImGui::Button(isPaused ? "Reprendre" : "Jouer")) {
                // Toujours utiliser Play() avec le son actuel pour s'assurer 
                // que la source utilise le bon buffer audio
                source->Play(currentSound, true);
            }
            ImGui::PopStyleColor();
        }
        
        if (isPlaying && !isPaused) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7f, 0.7f, 0.0f, 1.0f));
            if (ImGui::Button("Pause")) {
                source->Pause();
            }
            ImGui::PopStyleColor();
        }

        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7f, 0.0f, 0.0f, 1.0f));
        if (ImGui::Button("Arrêter")) {
            source->Stop();
        }
        ImGui::PopStyleColor();

        float ambientVolume = source->GetVolume();
        ImGui::Text("Volume Ambiance: %.0f%%", ambientVolume * 100.0f);
        if (ImGui::SliderFloat("##AmbientVolume", &ambientVolume, 0.0f, 1.0f, "%.2f")) {
            source->SetVolume(ambientVolume);
        }

        float ambientPitch = source->GetPitch();
        ImGui::Text("Pitch Ambiance: %.2fx", ambientPitch);
        if (ImGui::SliderFloat("##AmbientPitch", &ambientPitch, 0.5f, 2.0f, "%.2f")) {
            source->SetPitch(ambientPitch);
        }
    } else {
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Son d'ambiance non disponible");
        ImGui::Text("Vérifiez que les fichiers .wav sont dans le dossier sound/");
    }    ImGui::End();
}

void RenderMainControlsUI(SkyboxManager::SkyboxType& currentSkyboxType, std::function<void(SkyboxManager::SkyboxType)> onSkyboxChange) {
    ImGui::SetNextWindowPos(ImVec2(350, 10), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(280, 0), ImGuiCond_FirstUseEver);
    ImGui::Begin("Contrôles Principaux", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

    // Section Shaders
    ImGui::Text("Shader:");
    
    ShaderManager& shaderManager = ShaderManager::getInstance();
    LightingShaderType currentShaderType = shaderManager.GetLightingShaderType();
    
    static const LightingShaderType shaderTypes[] = {
        LightingShaderType::PHONG,
        LightingShaderType::LAMBERT
    };
    static const char* shaderNames[] = {
        "Phong",
        "Lambert"
    };
    
    int currentShaderIdx = 0;
    for (int i = 0; i < IM_ARRAYSIZE(shaderTypes); ++i) {
        if (shaderTypes[i] == currentShaderType) { 
            currentShaderIdx = i; 
            break; 
        }
    }
    
    ImGui::SetNextItemWidth(-1);
    if (ImGui::Combo("##ShaderCombo", &currentShaderIdx, shaderNames, IM_ARRAYSIZE(shaderNames))) {
        shaderManager.SetLightingShaderType(shaderTypes[currentShaderIdx]);
    }

    ImGui::Spacing();

    // Section Skybox
    ImGui::Text("Skybox:");
    
    static const SkyboxManager::SkyboxType skyboxTypes[] = {
        SkyboxManager::SkyboxType::COLORER,
        SkyboxManager::SkyboxType::DEFAULT,
        SkyboxManager::SkyboxType::SPACE,
        SkyboxManager::SkyboxType::ZOO
    };
    static const char* skyboxNames[] = {
        "Colorée",
        "Défaut",
        "Espace",
        "Zoo"
    };
    
    int currentSkyboxIdx = 0;
    for (int i = 0; i < IM_ARRAYSIZE(skyboxTypes); ++i) {
        if (skyboxTypes[i] == currentSkyboxType) { 
            currentSkyboxIdx = i; 
            break; 
        }
    }
    
    ImGui::SetNextItemWidth(-1);
    if (ImGui::Combo("##SkyboxCombo", &currentSkyboxIdx, skyboxNames, IM_ARRAYSIZE(skyboxNames))) {
        onSkyboxChange(skyboxTypes[currentSkyboxIdx]);
    }

    ImGui::End();
}

void RenderSkyboxUI(const char* title, SkyboxManager::SkyboxType& currentType, std::function<void(SkyboxManager::SkyboxType)> onChange) {
    ImGui::SetNextWindowPos(ImVec2(350, 10), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(300, 0), ImGuiCond_Always); // Largeur plus grande
    ImGui::Begin(title, nullptr, ImGuiWindowFlags_AlwaysAutoResize);
    static const SkyboxManager::SkyboxType types[] = {
        SkyboxManager::SkyboxType::COLORER,
        SkyboxManager::SkyboxType::DEFAULT,
        SkyboxManager::SkyboxType::SPACE,
        SkyboxManager::SkyboxType::ZOO
    };
    static const char* names[] = {
        "Skybox Colorée",
        "Skybox Par Défaut",
        "Skybox Spatiale",
        "Skybox Zoo"
    };
    int currentIdx = 0;
    for (int i = 0; i < IM_ARRAYSIZE(types); ++i) {
        if (types[i] == currentType) { currentIdx = i; break; }
    }
    ImGui::Text("Sélectionnez la skybox :");
    ImGui::SetNextItemWidth(250);
    if (ImGui::Combo("##SkyboxCombo", &currentIdx, names, IM_ARRAYSIZE(names))) {
        onChange(types[currentIdx]);
    }
    ImGui::End();
}

} // namespace UIHelpers
