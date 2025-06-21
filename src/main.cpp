#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>

#include "Shader.h"
#include "Camera.h"
#include "Model.h"
#include "Sphere.h"
#include "TextureLoader.h"
#include "SoundManager.h"
#include "Sound.h"
#include "AudioSource.h"
#include "SceneManager.h"
#include "MainScene.h"
#include "LightScene.h"
#include "UBO.h"
#include "ShaderManager.h"
#include <glm/gtc/matrix_transform.hpp>

// === ImGui ===
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

// === Modes de contrôle ===
enum ControlMode {
    CAMERA_MODE,    // Souris contrôle la caméra
    UI_MODE         // Souris disponible pour l'interface
};

// Mode de contrôle actuel
ControlMode currentMode = CAMERA_MODE;

// Dimensions de la fenêtre
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

// Caméra globale
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));

// Gestionnaire audio global
SoundManager soundManager;

// Gestionnaire de scènes global
SceneManager sceneManager;

// Timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// Variables pour la rotation de la sphère (pour compatibilité)
float luneOrbitRadius = 10.0f;
float luneOrbitSpeed = 0.25f;
float luneSelfRotSpeed = 1.0f;

// Position et taille du soleil (pour compatibilité)
float sunDistance = 100.0f;
float sunRadius = 45.0f;
glm::vec3 lightPosition = glm::vec3(-sunDistance, 15.0f, -sunDistance);

// Prototypes
void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void processInput(GLFWwindow *window);
void mouse_callback(GLFWwindow *window, double xpos, double ypos);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);

// Mouse variables
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// Variables audio (pour compatibilité avec l'ancien code)
std::shared_ptr<Sound> zooSound;
std::shared_ptr<AudioSource> ambientSource;

int main()
{
    // Initialisation GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Créer la fenêtre
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Projet OpenGL - Système de Scènes", NULL, NULL);
    if (window == NULL)
    {
        std::cerr << "Erreur : échec de la création de la fenêtre GLFW" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Initialisation GLEW
    if (glewInit() != GLEW_OK)
    {
        std::cerr << "Erreur : échec de l'initialisation de GLEW" << std::endl;
        return -1;
    }

    // Activer le depth test
    glEnable(GL_DEPTH_TEST);

    // === Initialisation ImGui ===
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Activer la navigation clavier
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");

    // === Initialisation du système UBO ===
    g_uboManager = new UBOManager();
    if (!g_uboManager->Initialize()) {
        std::cerr << "Erreur : échec de l'initialisation du système UBO" << std::endl;
        delete g_uboManager;
        g_uboManager = nullptr;
        glfwTerminate();
        return -1;
    }

    // === Initialisation du gestionnaire de shaders ===
    if (!ShaderManager::getInstance().Initialize()) {
        std::cerr << "Erreur : échec de l'initialisation du gestionnaire de shaders" << std::endl;
        delete g_uboManager;
        g_uboManager = nullptr;
        glfwTerminate();
        return -1;
    }

    // === Initialisation du système audio ===
    if (!soundManager.Initialize()) {
        std::cerr << "Erreur : échec de l'initialisation du système audio" << std::endl;
        // Continuer sans audio
    } else {
        // Charger tous les sons disponibles
        soundManager.LoadAllSounds();
        
        // Charger le son Zoo.wav (sera géré par les scènes)
        zooSound = soundManager.LoadSound("../sound/Zoo.wav", "zoo_ambient");

        if (zooSound) {
            // Créer une source audio pour l'ambiance
            ambientSource = soundManager.CreateAudioSource();
            if (ambientSource) {
                ambientSource->SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));
                ambientSource->SetVolume(0.3f);
                // Ne pas jouer automatiquement - sera contrôlé par l'interface
            }
        }
    }

    // === Initialisation du système de scènes ===
    // Créer et ajouter les scènes
    sceneManager.AddScene(std::make_unique<MainScene>());
    sceneManager.AddScene(std::make_unique<LightScene>());

    // Initialiser le gestionnaire de scènes
    if (!sceneManager.Initialize(camera, soundManager)) {
        std::cerr << "Erreur : échec de l'initialisation du gestionnaire de scènes" << std::endl;
        glfwTerminate();
        return -1;
    }

    std::cout << "Système de scènes initialisé avec " << sceneManager.GetSceneCount() << " scène(s)" << std::endl;
    std::cout << "Scène active : " << sceneManager.GetCurrentSceneName() << std::endl;

    // Configurer les callbacks de la souris
    glfwSetCursorPosCallback(window, [](GLFWwindow* w, double x, double y) {
        ImGui_ImplGlfw_CursorPosCallback(w, x, y); // transmet à ImGui
        mouse_callback(w, x, y); // appel perso
    });
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Réinitialiser firstMouse quand on change de mode
    firstMouse = true;

    std::cout << "=== CONTRÔLES ===" << std::endl;
    std::cout << "TAB: Basculer entre mode Caméra et Interface" << std::endl;
    std::cout << "P: Changer de scène" << std::endl;
    std::cout << "ZQSD/AE: Déplacer la caméra (mode caméra, AZERTY)" << std::endl;
    std::cout << "O: Arrêter tous les sons" << std::endl;
    std::cout << "ESC: Quitter" << std::endl;
    std::cout << "=================" << std::endl;

    // Boucle de rendu
    while (!glfwWindowShouldClose(window))
    {
        // Temps
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Input
        processInput(window);

        // Mettre à jour le système audio
        soundManager.Update();

        // Mettre à jour le gestionnaire de scènes
        sceneManager.Update(deltaTime, window, camera, soundManager);

        // === Mise à jour des UBOs ===
        if (g_uboManager) {
            // Matrices de projection et de vue
            glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), 
                                                   (float)SCR_WIDTH / (float)SCR_HEIGHT, 
                                                   0.1f, 1000.0f);
            glm::mat4 view = camera.GetViewMatrix();
            glm::vec3 viewPos = camera.Position;
            
            // Mise à jour du Camera UBO
            g_uboManager->UpdateCameraUBO(projection, view, viewPos);
            
            // Mise à jour du Lighting UBO avec des valeurs par défaut
            glm::vec3 lightPos(-100.0f, 15.0f, -100.0f);
            glm::vec3 lightColor(1.0f, 1.0f, 1.0f);
            glm::vec3 ambientColor(0.1f, 0.1f, 0.1f);
            g_uboManager->UpdateLightingUBO(lightPos, lightColor, ambientColor);
        }

        // Effacer l'écran
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Rendu de la scène actuelle
        sceneManager.Render(camera, SCR_WIDTH, SCR_HEIGHT);

        //======== INTERFACE IMGUI ========
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // === Interface de contrôle principal ===
        ImGui::SetNextWindowPos(ImVec2(990, 10), ImGuiCond_FirstUseEver);
        ImGui::Begin("Contrôles Principaux", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

        // Informations sur le mode actuel
        ImGui::Text("Mode actuel: %s", 
                   currentMode == CAMERA_MODE ? "Caméra" : "Interface");
        ImGui::Text("Scène actuelle: %s", sceneManager.GetCurrentSceneName());
        
        // Informations de debug caméra
        ImGui::Separator();
        ImGui::Text("Position caméra: (%.1f, %.1f, %.1f)", 
                   camera.Position.x, camera.Position.y, camera.Position.z);
        ImGui::Text("Direction caméra: (%.1f, %.1f, %.1f)", 
                   camera.Front.x, camera.Front.y, camera.Front.z);
          ImGui::Separator();
        
        // Instructions
        ImGui::Text("Contrôles:");
        ImGui::BulletText("TAB: Basculer mode Caméra/Interface");
        ImGui::BulletText("P: Changer de scène");
        ImGui::BulletText("ESC: Quitter");

        ImGui::Separator();

        // Boutons de contrôle rapide
        if (ImGui::Button(currentMode == CAMERA_MODE ? "Mode Interface" : "Mode Caméra")) {
            if (currentMode == CAMERA_MODE) {
                currentMode = UI_MODE;
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                firstMouse = true; // Réinitialiser pour éviter les sauts
            } else {
                currentMode = CAMERA_MODE;
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                firstMouse = true; // Réinitialiser pour éviter les sauts
            }
        }
        
        ImGui::SameLine();
        if (ImGui::Button("Scène Suivante")) {
            sceneManager.NextScene();
        }        ImGui::End();

        // Rendu de l'interface de la scène actuelle
        sceneManager.RenderUI(window, soundManager);

        // rendu ImGui
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Affichage
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // === Nettoyage ===
    sceneManager.Cleanup();
    soundManager.Shutdown();
    
    // Nettoyage du gestionnaire de shaders
    ShaderManager::getInstance().Cleanup();
    
    // Nettoyage du système UBO
    if (g_uboManager) {
        delete g_uboManager;
        g_uboManager = nullptr;
    }

    // === Nettoyage ImGui ===
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwTerminate();
    return 0;
}

// Resize callback
void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
}

// Clavier
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // Gestion de la vitesse de déplacement avec Shift
    float speedMultiplier = 1.0f;
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ||
        glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS) {
        speedMultiplier = 10.0f;
    }

    // Contrôles de la caméra (seulement en mode caméra) - Configuration AZERTY
    if (currentMode == CAMERA_MODE) {
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)  // W = Avant (Z sur AZERTY)
            camera.ProcessKeyboard(FORWARD, deltaTime * speedMultiplier);
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)  // S = Arrière
            camera.ProcessKeyboard(BACKWARD, deltaTime * speedMultiplier);
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)  // A = Gauche (Q sur AZERTY)
            camera.ProcessKeyboard(LEFT, deltaTime * speedMultiplier);
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)  // D = Droite
            camera.ProcessKeyboard(RIGHT, deltaTime * speedMultiplier);
        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)  // Q = Monter (A sur AZERTY)
            camera.ProcessKeyboard(UP, deltaTime * speedMultiplier);
        if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)  // E = Descendre
            camera.ProcessKeyboard(DOWN, deltaTime * speedMultiplier);
    }

    // Variables statiques pour éviter les répétitions
    static bool pPressed = false;
    static bool oPressed = false;
    static bool musicPlaying = false;
    static bool tabPressed = false;

    // Touche TAB pour basculer entre les modes
    if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS && !tabPressed) {
        tabPressed = true;
        if (currentMode == CAMERA_MODE) {
            currentMode = UI_MODE;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            firstMouse = true; // Réinitialiser pour éviter les sauts
            std::cout << "Mode Interface activé - Souris disponible pour l'UI" << std::endl;
        } else {
            currentMode = CAMERA_MODE;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            firstMouse = true; // Réinitialiser pour éviter les sauts
            std::cout << "Mode Caméra activé - Souris contrôle la caméra" << std::endl;
        }
    } else if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_RELEASE) {
        tabPressed = false;
    }

    // Touche P pour changer de scène
    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS && !pPressed) {
        pPressed = true;
        sceneManager.NextScene();
    } else if (glfwGetKey(window, GLFW_KEY_P) == GLFW_RELEASE) {
        pPressed = false;
    }



    // Touche O pour arrêter tous les sons ou les redémarrer
    if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS && !oPressed) {
        oPressed = true;
        if (musicPlaying) {
            soundManager.ResumeAll();
            musicPlaying = false;
        } else {
            soundManager.StopAll();
            musicPlaying = true;
        }
    } else if (glfwGetKey(window, GLFW_KEY_O) == GLFW_RELEASE) {
        oPressed = false;
        soundManager.ResumeAll();
    }
}

// Souris
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    // En mode interface, laisser ImGui gérer la souris
    if (currentMode == UI_MODE) {
        return;
    }

    // Mode caméra : traiter les mouvements de souris pour la caméra
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed: y ranges from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    // Ne traiter le scroll qu'en mode caméra
    if (currentMode == CAMERA_MODE) {
        camera.ProcessMouseScroll(static_cast<float>(yoffset));
    }
}
