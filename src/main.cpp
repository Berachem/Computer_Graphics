#include <GL/glew.h> // Remplacé glad par glew
#include <GLFW/glfw3.h>
#include <iostream>

#include "Shader.h"
#include "Camera.h"
#include "Model.h"
#include "Sphere.h"
#include "TextureLoader.h"
#include <glm/gtc/matrix_transform.hpp>

// === ImGui ===
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

// Dimensions de la fenêtre
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

// Caméra globale
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));

// Timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// Variables pour la rotation de la sphère
float luneOrbitRadius = 10.0f;     // Distance de la sphère par rapport à l'avion
float luneOrbitSpeed = 0.25f;      // Vitesse de rotation autour de l'avion
float luneSelfRotSpeed = 1.0f;    // Vitesse de rotation sur elle-même

// Position et taille du soleil
float sunDistance = 50.0f;         // Distance du soleil
float sunRadius = 10.0f;           // Rayon du soleil
glm::vec3 lightPosition = glm::vec3(-sunDistance, 15.0f, -sunDistance); // Position de la lumière (même que le soleil)

// Prototypes
void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void processInput(GLFWwindow *window);
void mouse_callback(GLFWwindow *window, double xpos, double ypos);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
void updateSunPosition(float time);

// Mouse variables
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

int main()
{
    // Initialiser GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Créer la fenêtre
    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Projet OpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cerr << "Erreur : échec de création de la fenêtre GLFW" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Initialiser GLEW (à la place de GLAD)
    glewExperimental = GL_TRUE;
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
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");

    // Charger les shaders
    Shader simpleShader("../shaders/simple_color.vert", "../shaders/simple_color.frag");
    Shader phongShader("../shaders/phong.vert", "../shaders/phong.frag");
    Shader texturedShader("../shaders/textured.vert", "../shaders/textured.frag");
    Shader sunShader("../shaders/sun.vert", "../shaders/sun.frag");

    // Charger un modèle
    Model myModel("../models/map-bump.obj");

    // Créer une sphère texturée (texture, rayon, secteurs, stacks)
    Sphere moonSphere("../textures/spherical_moon_texture.jpg", 0.5f, 36, 18);
    
    // Créer une sphère pour le soleil (pas de texture, juste une couleur uniforme)
    Sphere sunSphere("", sunRadius, 36, 18);

    // Configurer les callbacks de la souris
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Boucle de rendu
    while (!glfwWindowShouldClose(window))
    {
        // Temps
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Input
        processInput(window);
        
        // Mise à jour de la position du soleil
        updateSunPosition(currentFrame);

        // Effacer l'écran
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Activer le shader
        phongShader.use();

        // Matrices
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        phongShader.setMat4("projection", projection);
        phongShader.setMat4("view", view);
        
        // Définir la position de la lumière pour le shader phong
        phongShader.setVec3("lightPos", lightPosition);
        phongShader.setVec3("viewPos", camera.Position);

        // Transformation du modèle d'avion
        glm::mat4 model = glm::mat4(1.0f);
        phongShader.setMat4("model", model);

        // Dessiner l'avion
        myModel.Draw(phongShader);

        // Calculer la position de la sphère (rotation autour de l'avion)
        float orbitAngle = currentFrame * luneOrbitSpeed;
        float sphereX = luneOrbitRadius * cos(orbitAngle);
        float sphereZ = luneOrbitRadius * sin(orbitAngle);

        // Transformation de la sphère (lune)
        glm::mat4 sphereModel = glm::mat4(1.0f);
        // Translation à la position orbitale
        sphereModel = glm::translate(sphereModel, glm::vec3(sphereX, 2.0f, sphereZ));
        // Rotation sur elle-même
        sphereModel = glm::rotate(sphereModel, currentFrame * luneSelfRotSpeed, glm::vec3(0.0f, 1.0f, 0.0f));

        // Activer le shader texturé pour la lune
        texturedShader.use();
        texturedShader.setMat4("projection", projection);
        texturedShader.setMat4("view", view);
        texturedShader.setMat4("model", sphereModel);
        texturedShader.setVec3("lightPos", lightPosition); // Utiliser la position du soleil comme source de lumière
        texturedShader.setVec3("viewPos", camera.Position);

        // Dessiner la sphère texturée (lune)
        moonSphere.Draw(texturedShader);

        // Dessiner le soleil avec le nouveau shader
        sunShader.use();
        sunShader.setMat4("projection", projection);
        sunShader.setMat4("view", view);
        sunShader.setFloat("time", currentFrame);  // Passage du temps au shader
        
        // Positionner le soleil loin dans la scène
        glm::mat4 sunModel = glm::mat4(1.0f);
        sunModel = glm::translate(sunModel, lightPosition);
        sunShader.setMat4("model", sunModel);
        
        sunSphere.Draw(sunShader);
        
        // Revenir au shader phong pour les autres objets
        phongShader.use();
        phongShader.setMat4("model", model);

        // début de frame ImGui…
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // --- juste avant ImGui::Begin("Controls",…) ---
        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(
            ImVec2(viewport->WorkPos.x + 10, 
                viewport->WorkPos.y + viewport->WorkSize.y - 10),
            ImGuiCond_Always,
            ImVec2(0.0f, 1.0f)       // pivot = coin bas-gauche de la fenêtre
        );

        ImGui::Begin("Controls", nullptr,
                    ImGuiWindowFlags_NoDecoration |
                    ImGuiWindowFlags_AlwaysAutoResize |
                    ImGuiWindowFlags_NoBackground);

        // taille et espacement
        const float keySize = 40.0f;
        const float sp     = ImGui::GetStyle().ItemSpacing.x;

        // petite lambda pour dessiner une touche « caps »
        auto drawKey = [&](const char* label, int glfw_key){
            bool down = (glfwGetKey(window, glfw_key) == GLFW_PRESS);
            ImVec4 col = down 
            ? ImVec4(0.2f,0.8f,0.2f,1.0f) 
            : ImGui::GetStyleColorVec4(ImGuiCol_Button);
            ImGui::PushStyleColor(ImGuiCol_Button,         col);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered,  ImVec4(col.x+0.1f, col.y+0.1f, col.z+0.1f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive,   ImVec4(col.x-0.1f, col.y-0.1f, col.z-0.1f, 1.0f));
            ImGui::Button(label, ImVec2(keySize, keySize));
            ImGui::PopStyleColor(3);
        };

        // 1️⃣ Première ligne : on centre “Z”
        ImGui::Dummy(ImVec2(keySize, 0.0f)); ImGui::SameLine();
        drawKey("Z", GLFW_KEY_W);
        ImGui::SameLine((3*keySize + 2*sp) + sp*5);
        drawKey("A", GLFW_KEY_Q);

        //ImGui::NewLine();

        // 2️⃣ Deuxième ligne : Q, S, D
        drawKey("Q", GLFW_KEY_A); ImGui::SameLine();
        drawKey("S", GLFW_KEY_S); ImGui::SameLine();
        drawKey("D", GLFW_KEY_D);
        // et, juste à leur droite, les touche A et E
        //ImGui::SameLine((3*keySize + 2*sp) + sp);
        ImGui::SameLine((3*keySize + 2*sp) + sp*5);
        drawKey("E", GLFW_KEY_E);

        ImGui::End();


        // rendu ImGui…
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());


        // Affichage
        glfwSwapBuffers(window);
        glfwPollEvents();
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

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        camera.ProcessKeyboard(UP, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        camera.ProcessKeyboard(DOWN, deltaTime);
}

// Souris
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
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
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

// Mise à jour de la position du soleil si nécessaire (exemple avec rotation)
void updateSunPosition(float time) {
    float angle = time * 0.0005f; // Très lente rotation
    lightPosition.x = -sunDistance * cos(angle);
    lightPosition.z = -sunDistance * sin(angle);
    lightPosition.y = 15.0f;
}
