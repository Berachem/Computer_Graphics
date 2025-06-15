#include "MainScene.h"
#include "imgui.h"
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>

// Variables pour les animations (extraites de main.cpp)
static float luneOrbitRadius = 10.0f;
static float luneOrbitSpeed = 0.25f;
static float luneSelfRotSpeed = 1.0f;
static float sunDistance = 100.0f;
static glm::vec3 lightPosition = glm::vec3(-100.0f, 15.0f, -100.0f);

MainScene::MainScene() 
    : sunRadius(45.0f), initialized(false) {
}

MainScene::~MainScene() {
    Cleanup();
}

bool MainScene::Initialize(Camera& camera, SoundManager& soundManager) {
    if (initialized) {
        return true;
    }

    std::cout << "Initialisation de la MainScene..." << std::endl;

    // Charger les shaders
    if (!LoadShaders()) {
        std::cerr << "Erreur : échec du chargement des shaders" << std::endl;
        return false;
    }

    // Charger les modèles
    if (!LoadModels()) {
        std::cerr << "Erreur : échec du chargement des modèles" << std::endl;
        return false;
    }

    // Charger l'audio
    if (!LoadAudio(soundManager)) {
        std::cerr << "Avertissement : échec du chargement de l'audio" << std::endl;
        // Continuer sans audio
    }

    initialized = true;
    std::cout << "MainScene initialisée avec succès" << std::endl;
    return true;
}

bool MainScene::LoadShaders() {
    try {
        simpleShader = std::make_unique<Shader>("../shaders/simple_color.vert", "../shaders/simple_color.frag");
        phongShader = std::make_unique<Shader>("../shaders/phong.vert", "../shaders/phong.frag");
        texturedShader = std::make_unique<Shader>("../shaders/textured.vert", "../shaders/textured.frag");
        sunShader = std::make_unique<Shader>("../shaders/sun.vert", "../shaders/sun.frag");
        metalShader = std::make_unique<Shader>("../shaders/metal.vert", "../shaders/metal.frag");
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Erreur lors du chargement des shaders : " << e.what() << std::endl;
        return false;
    }
}

bool MainScene::LoadModels() {
    try {
        myModel = std::make_unique<Model>("../models/map-bump.obj");
        asteroid1 = std::make_unique<Model>("../models/astroid.obj");
        asteroid2 = std::make_unique<Model>("../models/astroid.obj");
        asteroid3 = std::make_unique<Model>("../models/astroid.obj");
        asteroid4 = std::make_unique<Model>("../models/astroid.obj");
        
        moonSphere = std::make_unique<Sphere>("../textures/spherical_moon_texture.jpg", 0.5f, 36, 18);
        sunSphere = std::make_unique<Sphere>("", sunRadius, 36, 18);
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Erreur lors du chargement des modèles : " << e.what() << std::endl;
        return false;
    }
}

bool MainScene::LoadAudio(SoundManager& soundManager) {
    if (!soundManager.IsInitialized()) {
        return false;
    }

    zooSound = soundManager.LoadSound("../sound/Zoo.wav", "zoo_ambient");
    if (zooSound) {
        ambientSource = soundManager.CreateAudioSource();
        if (ambientSource) {
            ambientSource->SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));
            ambientSource->SetVolume(0.3f);
            // Ne pas jouer automatiquement - sera contrôlé par l'interface
            return true;
        }
    }
    return false;
}

void MainScene::Update(float deltaTime, GLFWwindow* window, Camera& camera, SoundManager& soundManager) {
    if (!initialized) return;

    // Mettre à jour la position de l'auditeur avec la caméra
    if (soundManager.IsInitialized()) {
        float listenerPos[3] = {camera.Position.x, camera.Position.y, camera.Position.z};
        float listenerForward[3] = {camera.Front.x, camera.Front.y, camera.Front.z};
        float listenerUp[3] = {camera.Up.x, camera.Up.y, camera.Up.z};
        soundManager.SetListenerPosition(listenerPos, listenerForward, listenerUp);
    }

    // Mettre à jour la position du soleil (rotation lente)
    float currentFrame = static_cast<float>(glfwGetTime());
    float angle = currentFrame * 0.0005f;
    lightPosition.x = -sunDistance * cos(angle);
    lightPosition.z = -sunDistance * sin(angle);
    lightPosition.y = 15.0f;
}

void MainScene::Render(Camera& camera, int screenWidth, int screenHeight) {
    if (!initialized) return;

    RenderObjects(camera, screenWidth, screenHeight);
}

void MainScene::RenderUI(GLFWwindow* window, SoundManager& soundManager) {
    if (!initialized) return;

    // Interface audio uniquement pour la scène principale
    RenderAudioUI(window, soundManager);
    // Interface clavier pour visualiser les touches
    RenderKeyboardUI(window);
}

const char* MainScene::GetName() const {
    return "Scène Principale";
}

void MainScene::OnActivate() {
    std::cout << "MainScene activée" << std::endl;
    // Reprendre l'audio si nécessaire
    if (ambientSource && zooSound && !ambientSource->IsPlaying()) {
        ambientSource->Play(zooSound, true);
    }
}

void MainScene::OnDeactivate() {
    std::cout << "MainScene désactivée" << std::endl;
    // Optionnel : mettre en pause l'audio
}

void MainScene::RenderObjects(Camera& camera, int screenWidth, int screenHeight) {
    float currentFrame = static_cast<float>(glfwGetTime());

    // Matrices de projection et de vue communes
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom),
                                          (float)screenWidth / (float)screenHeight,
                                          0.1f, 1000.0f);
    glm::mat4 view = camera.GetViewMatrix();

    //======== VAISSEAU ========
    metalShader->use();
    metalShader->setMat4("projection", projection);
    metalShader->setMat4("view", view);
    metalShader->setVec3("lightPos", lightPosition);
    metalShader->setVec3("viewPos", camera.Position);

    glm::mat4 model = glm::mat4(1.0f);
    metalShader->setMat4("model", model);
    myModel->Draw(*metalShader);

    //======== LUNE ========
    texturedShader->use();
    texturedShader->setMat4("projection", projection);
    texturedShader->setMat4("view", view);
    texturedShader->setVec3("lightPos", lightPosition);
    texturedShader->setVec3("viewPos", camera.Position);

    // Orbite de la lune
    float orbitAngle = currentFrame * luneOrbitSpeed;
    float sphereX = luneOrbitRadius * cos(orbitAngle);
    float sphereZ = luneOrbitRadius * sin(orbitAngle);

    glm::mat4 sphereModel = glm::mat4(1.0f);
    sphereModel = glm::translate(sphereModel, glm::vec3(sphereX, 2.0f, sphereZ));
    sphereModel = glm::rotate(sphereModel, currentFrame * luneSelfRotSpeed, glm::vec3(0.0f, 1.0f, 0.0f));
    texturedShader->setMat4("model", sphereModel);
    moonSphere->Draw(*texturedShader);

    //======== SOLEIL ========
    sunShader->use();
    sunShader->setMat4("projection", projection);
    sunShader->setMat4("view", view);
    sunShader->setFloat("time", currentFrame);

    glm::mat4 sunModel = glm::mat4(1.0f);
    sunModel = glm::translate(sunModel, lightPosition);
    sunShader->setMat4("model", sunModel);
    sunSphere->Draw(*sunShader);

    //======== ASTEROIDS ========
    phongShader->use();
    phongShader->setVec3("lightPos", lightPosition);
    phongShader->setVec3("viewPos", camera.Position);
    phongShader->setMat4("projection", projection);
    phongShader->setMat4("view", view);

    float asteroidOrbitAngle = currentFrame * 0.5f;
    float asteroidOrbitRadius = 60.0f;

    // Astéroïde 1
    float inclinationAngle = glm::radians(15.0f);
    glm::mat4 inclinationMatrix = glm::rotate(glm::mat4(1.0f), inclinationAngle, glm::vec3(1.0f, 0.0f, 0.0f));
    glm::vec3 asteroidPosition = glm::vec3(
        asteroidOrbitRadius * cos(asteroidOrbitAngle),
        0.0f,
        asteroidOrbitRadius * sin(asteroidOrbitAngle)
    );
    glm::vec4 inclinedPosition = inclinationMatrix * glm::vec4(asteroidPosition, 1.0f);

    glm::mat4 asteroidModel = glm::mat4(1.0f);
    asteroidModel = glm::translate(asteroidModel, glm::vec3(lightPosition.x + inclinedPosition.x,
                                                           lightPosition.y + inclinedPosition.y,
                                                           lightPosition.z + inclinedPosition.z));
    asteroidModel = glm::scale(asteroidModel, glm::vec3(0.05f, 0.05f, 0.05f));
    asteroidModel = glm::rotate(asteroidModel, asteroidOrbitAngle * 10.0f, glm::vec3(0.0f, 1.0f, 0.5f));
    phongShader->setMat4("model", asteroidModel);
    asteroid1->Draw(*phongShader);

    // Astéroïde 2
    inclinationAngle = glm::radians(11.0f);
    inclinationMatrix = glm::rotate(glm::mat4(1.0f), inclinationAngle, glm::vec3(1.0f, 0.0f, 0.0f));
    glm::vec3 asteroid2Position = glm::vec3(
        asteroidOrbitRadius * cos(asteroidOrbitAngle + glm::radians(10.0f)),
        0.0f,
        asteroidOrbitRadius * sin(asteroidOrbitAngle + glm::radians(10.0f))
    );
    glm::vec4 inclinedPosition2 = inclinationMatrix * glm::vec4(asteroid2Position, 1.0f);

    glm::mat4 asteroid2Model = glm::mat4(1.0f);
    asteroid2Model = glm::translate(asteroid2Model, glm::vec3(lightPosition.x + inclinedPosition2.x,
                                                             lightPosition.y + inclinedPosition2.y,
                                                             lightPosition.z + inclinedPosition2.z));
    asteroid2Model = glm::scale(asteroid2Model, glm::vec3(0.025f, 0.042f, 0.015f));
    asteroid2Model = glm::rotate(asteroid2Model, asteroidOrbitAngle * 7.0f, glm::vec3(0.5f, 1.0f, 0.0f));
    phongShader->setMat4("model", asteroid2Model);
    asteroid2->Draw(*phongShader);

    // Astéroïde 3
    inclinationAngle = glm::radians(0.0f);
    inclinationMatrix = glm::rotate(glm::mat4(1.0f), inclinationAngle, glm::vec3(1.0f, 0.0f, 0.0f));
    glm::vec3 asteroid3Position = glm::vec3(
        asteroidOrbitRadius * cos(asteroidOrbitAngle + glm::radians(15.0f)),
        0.0f,
        asteroidOrbitRadius * sin(asteroidOrbitAngle + glm::radians(15.0f))
    );
    glm::vec4 inclinedPosition3 = inclinationMatrix * glm::vec4(asteroid3Position, 1.0f);

    glm::mat4 asteroid3Model = glm::mat4(1.0f);
    asteroid3Model = glm::translate(asteroid3Model, glm::vec3(lightPosition.x + inclinedPosition3.x,
                                                             lightPosition.y + inclinedPosition3.y,
                                                             lightPosition.z + inclinedPosition3.z));
    asteroid3Model = glm::scale(asteroid3Model, glm::vec3(0.08f, 0.08f, 0.08f));
    asteroid3Model = glm::rotate(asteroid3Model, asteroidOrbitAngle * 12.0f, glm::vec3(1.0f, 0.0f, 1.0f));
    phongShader->setMat4("model", asteroid3Model);
    asteroid3->Draw(*phongShader);

    // Astéroïde 4
    inclinationAngle = glm::radians(8.0f);
    inclinationMatrix = glm::rotate(glm::mat4(1.0f), inclinationAngle, glm::vec3(1.0f, 0.0f, 0.0f));
    glm::vec3 asteroid4Position = glm::vec3(
        asteroidOrbitRadius * cos(asteroidOrbitAngle + glm::radians(25.0f)),
        0.0f,
        asteroidOrbitRadius * sin(asteroidOrbitAngle + glm::radians(25.0f))
    );
    glm::vec4 inclinedPosition4 = inclinationMatrix * glm::vec4(asteroid4Position, 1.0f);

    glm::mat4 asteroid4Model = glm::mat4(1.0f);
    asteroid4Model = glm::translate(asteroid4Model, glm::vec3(lightPosition.x + inclinedPosition4.x,
                                                             lightPosition.y + inclinedPosition4.y,
                                                             lightPosition.z + inclinedPosition4.z));
    asteroid4Model = glm::scale(asteroid4Model, glm::vec3(0.012f, 0.014f, 0.013f));
    asteroid4Model = glm::rotate(asteroid4Model, asteroidOrbitAngle * 5.0f, glm::vec3(0.3f, 0.7f, 0.2f));
    phongShader->setMat4("model", asteroid4Model);
    asteroid4->Draw(*phongShader);
}

void MainScene::RenderAudioUI(GLFWwindow* window, SoundManager& soundManager) {
    if (!soundManager.IsInitialized()) return;

    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
    ImGui::Begin("Contrôles Audio", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

    // Volume principal avec affichage de la valeur
    float masterVolume = soundManager.GetMasterVolume();
    ImGui::Text("Volume Principal: %.0f%%", masterVolume * 100.0f);
    if (ImGui::SliderFloat("##MasterVolume", &masterVolume, 0.0f, 1.0f, "%.2f")) { 
        soundManager.SetMasterVolume(masterVolume);
    }

    ImGui::Separator();

    // Contrôles pour le son d'ambiance
    if (zooSound && ambientSource) {
        ImGui::Text("Son d'ambiance: %s", zooSound->GetFileName().c_str());
        ImGui::Text("Durée: %.1fs", zooSound->GetDuration());

        // État avec couleur
        bool isPlaying = ambientSource->IsPlaying();
        bool isPaused = ambientSource->IsPaused();

        if (isPlaying) {
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "État: En cours");
        } else if (isPaused) {
            ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "État: En pause");
        } else {
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "État: Arrêté");
        }

        // Boutons avec couleurs - Logique corrigée pour la pause/reprise
        if (!isPlaying && !isPaused) {
            // État arrêté - Bouton Jouer
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.7f, 0.0f, 1.0f));
            if (ImGui::Button("Jouer")) {
                ambientSource->Play(zooSound, true);
            }
            ImGui::PopStyleColor();
        } else if (isPlaying) {
            // État en cours - Bouton Pause
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7f, 0.7f, 0.0f, 1.0f));
            if (ImGui::Button("Pause")) {
                ambientSource->Pause();
            }
            ImGui::PopStyleColor();
        } else if (isPaused) {
            // État en pause - Bouton Reprendre
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.7f, 0.0f, 1.0f));
            if (ImGui::Button("Reprendre")) {
                ambientSource->Resume();
            }
            ImGui::PopStyleColor();
        }

        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7f, 0.0f, 0.0f, 1.0f));
        if (ImGui::Button("Arrêter")) {
            ambientSource->Stop();
        }
        ImGui::PopStyleColor();

        // Volume de l'ambiance avec affichage
        float ambientVolume = ambientSource->GetVolume();
        ImGui::Text("Volume Ambiance: %.0f%%", ambientVolume * 100.0f);
        if (ImGui::SliderFloat("##AmbientVolume", &ambientVolume, 0.0f, 1.0f, "%.2f")) {
            ambientSource->SetVolume(ambientVolume);
        }

        // Pitch de l'ambiance avec affichage
        float ambientPitch = ambientSource->GetPitch();
        ImGui::Text("Pitch Ambiance: %.2fx", ambientPitch);
        if (ImGui::SliderFloat("##AmbientPitch", &ambientPitch, 0.5f, 2.0f, "%.2f")) {
            ambientSource->SetPitch(ambientPitch);
        }
    } else {
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Son d'ambiance non disponible");
        ImGui::Text("Vérifiez que Zoo.wav est dans le dossier sound/");
    }



    ImGui::End();
}

void MainScene::RenderKeyboardUI(GLFWwindow* window) {
    // Interface clavier AZERTY
    ImGui::SetNextWindowPos(ImVec2(10, 500), ImGuiCond_FirstUseEver);
    ImGui::Begin("Clavier Virtuel (AZERTY)", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

    const float keySize = 40.0f;
    const float sp = ImGui::GetStyle().ItemSpacing.x;

    // Lambda pour dessiner une touche
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

    // Première ligne - W (avant sur clavier, Z sur AZERTY)
    ImGui::Dummy(ImVec2(keySize, 0.0f)); ImGui::SameLine(0, sp);
    drawKey("Z", GLFW_KEY_W, "Avant (Z sur AZERTY)"); ImGui::SameLine(0, sp);
    ImGui::Dummy(ImVec2(keySize, 0.0f)); ImGui::SameLine(0, sp*3);
    drawKey("A", GLFW_KEY_Q, "Monter (A sur AZERTY)");

    // Deuxième ligne - A S D (gauche, arrière, droite)
    drawKey("Q", GLFW_KEY_A, "Gauche (Q sur AZERTY)"); ImGui::SameLine(0, sp);
    drawKey("S", GLFW_KEY_S, "Arrière"); ImGui::SameLine(0, sp);
    drawKey("D", GLFW_KEY_D, "Droite"); ImGui::SameLine(0, sp*3);
    drawKey("E", GLFW_KEY_E, "Descendre");

    ImGui::Separator();
    ImGui::Text("Contrôles:");

    // Troisième ligne - Contrôles
    drawKey("TAB", GLFW_KEY_TAB, "Basculer mode"); ImGui::SameLine(0, sp);
    drawKey("P", GLFW_KEY_P, "Changer scène"); ImGui::SameLine(0, sp);
    drawKey("O", GLFW_KEY_O, "Arrêter sons");

    ImGui::End();
}

void MainScene::Cleanup() {
    if (ambientSource) {
        ambientSource->Stop();
    }

    // Les smart pointers se nettoient automatiquement
    simpleShader.reset();
    phongShader.reset();
    texturedShader.reset();
    sunShader.reset();
    metalShader.reset();

    myModel.reset();
    asteroid1.reset();
    asteroid2.reset();
    asteroid3.reset();
    asteroid4.reset();

    moonSphere.reset();
    sunSphere.reset();

    initialized = false;
    std::cout << "MainScene nettoyée" << std::endl;
}
