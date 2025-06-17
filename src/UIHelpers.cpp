#include "UIHelpers.h"
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

    if (sound && source) {
        ImGui::Text("Son d'ambiance: %s", sound->GetFileName().c_str());
        ImGui::Text("Durée: %.1fs", sound->GetDuration());

        bool isPlaying = source->IsPlaying();
        bool isPaused = source->IsPaused();

        if (isPlaying) {
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "État: En cours");
        } else if (isPaused) {
            ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "État: En pause");
        } else {
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "État: Arrêté");
        }

        if (!isPlaying && !isPaused) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.7f, 0.0f, 1.0f));
            if (ImGui::Button("Jouer")) {
                source->Play(sound, true);
            }
            ImGui::PopStyleColor();
        } else if (isPlaying) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7f, 0.7f, 0.0f, 1.0f));
            if (ImGui::Button("Pause")) {
                source->Pause();
            }
            ImGui::PopStyleColor();
        } else if (isPaused) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.7f, 0.0f, 1.0f));
            if (ImGui::Button("Reprendre")) {
                source->Resume();
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
        ImGui::Text("Vérifiez que Zoo.wav est dans le dossier sound/");
    }

    ImGui::End();
}

void RenderKeyboardUI(GLFWwindow* window) {
    ImGui::SetNextWindowPos(ImVec2(10, 500), ImGuiCond_FirstUseEver);
    ImGui::Begin("Clavier Virtuel (AZERTY)", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

    const float keySize = 40.0f;
    const float sp = ImGui::GetStyle().ItemSpacing.x;

    auto drawKey = [&](const char* label, int glfw_key, const char* description = nullptr) {
        bool down = (glfwGetKey(window, glfw_key) == GLFW_PRESS);
        ImVec4 col = down
            ? ImVec4(0.2f, 0.8f, 0.2f, 1.0f)
            : ImGui::GetStyleColorVec4(ImGuiCol_Button);
        ImGui::PushStyleColor(ImGuiCol_Button, col);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(col.x+0.1f, col.y+0.1f, col.z+0.1f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(col.x-0.1f, col.y-0.1f, col.z-0.1f, 1.0f));
        ImGui::Button(label, ImVec2(keySize, keySize));
        ImGui::PopStyleColor(3);

        if (description && ImGui::IsItemHovered()) {
            ImGui::SetTooltip("%s", description);
        }
    };

    ImGui::Text("Déplacement caméra:");

    ImGui::Dummy(ImVec2(keySize, 0.0f)); ImGui::SameLine(0, sp);
    drawKey("Z", GLFW_KEY_W, "Avant (Z sur AZERTY)"); ImGui::SameLine(0, sp);
    ImGui::Dummy(ImVec2(keySize, 0.0f)); ImGui::SameLine(0, sp*3);
    drawKey("A", GLFW_KEY_Q, "Monter (A sur AZERTY)");

    drawKey("Q", GLFW_KEY_A, "Gauche (Q sur AZERTY)"); ImGui::SameLine(0, sp);
    drawKey("S", GLFW_KEY_S, "Arrière"); ImGui::SameLine(0, sp);
    drawKey("D", GLFW_KEY_D, "Droite"); ImGui::SameLine(0, sp*3);
    drawKey("E", GLFW_KEY_E, "Descendre");

    ImGui::Separator();
    ImGui::Text("Contrôles:");

    drawKey("TAB", GLFW_KEY_TAB, "Basculer mode"); ImGui::SameLine(0, sp);
    drawKey("P", GLFW_KEY_P, "Changer scène"); ImGui::SameLine(0, sp);
    drawKey("O", GLFW_KEY_O, "Arrêter sons");

    ImGui::End();
}

} // namespace UIHelpers
