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
float sunDistance = 100.0f;         // Distance du soleil
float sunRadius = 45.0f;           // Rayon du soleil
glm::vec3 lightPosition = glm::vec3(-sunDistance, 15.0f, -sunDistance); // Position de la lumière (même que le soleil)

// Prototypes
void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void processInput(GLFWwindow *window);
void mouse_callback(GLFWwindow *window, double xpos, double ypos);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);

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
    Shader metalShader("../shaders/metal.vert", "../shaders/metal.frag");  // Nouveau shader métal

    // Charger un modèle
    Model myModel("../models/map-bump.obj");
   

    // Créer une sphère texturée (texture, rayon, secteurs, stacks)
    Sphere moonSphere("../textures/spherical_moon_texture.jpg", 0.5f, 36, 18);
    
    // Créer une sphère pour le soleil (pas de texture, juste une couleur uniforme)
    Sphere sunSphere("", sunRadius, 36, 18);

    //asteroids
    Model asteroid1("../models/astroid.obj");
    Model asteroid2("../models/astroid.obj");
    Model asteroid3("../models/astroid.obj");
    Model asteroid4("../models/astroid.obj"); 

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

        // Effacer l'écran
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //======== VAISSEAU ========

        //shader
        metalShader.use();
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        metalShader.setMat4("projection", projection);
        metalShader.setMat4("view", view);
        metalShader.setVec3("lightPos", lightPosition);
        metalShader.setVec3("viewPos", camera.Position);

        //position
        glm::mat4 model = glm::mat4(1.0f);
        metalShader.setMat4("model", model);

        //draw
        myModel.Draw(metalShader);

        //======== LUNE ========

        //shader
        texturedShader.use();
        texturedShader.setMat4("projection", projection);
        texturedShader.setMat4("view", view);
        texturedShader.setVec3("lightPos", lightPosition); // Utiliser la position du soleil comme source de lumière
        texturedShader.setVec3("viewPos", camera.Position);

        //orbite
        float orbitAngle = currentFrame * luneOrbitSpeed;
        float sphereX = luneOrbitRadius * cos(orbitAngle);
        float sphereZ = luneOrbitRadius * sin(orbitAngle);

        //position
        glm::mat4 sphereModel = glm::mat4(1.0f);
        sphereModel = glm::translate(sphereModel, glm::vec3(sphereX, 2.0f, sphereZ));
        sphereModel = glm::rotate(sphereModel, currentFrame * luneSelfRotSpeed, glm::vec3(0.0f, 1.0f, 0.0f));
        texturedShader.setMat4("model", sphereModel);

        //draw
        moonSphere.Draw(texturedShader);

        //======== SOLEIL ========

        //shader
        sunShader.use();
        sunShader.setMat4("projection", projection);
        sunShader.setMat4("view", view);
        sunShader.setFloat("time", currentFrame);

        //orbite
        float angle = currentFrame * 0.0005f; // Très lente rotation
        lightPosition.x = -sunDistance * cos(angle);
        lightPosition.z = -sunDistance * sin(angle);
        lightPosition.y = 15.0f;

        //position
        glm::mat4 sunModel = glm::mat4(1.0f);
        sunModel = glm::translate(sunModel, lightPosition);
        sunShader.setMat4("model", sunModel);
        
        //draw
        sunSphere.Draw(sunShader);
        
        //======== ASTEROIDS ========
        
        //shader
        phongShader.use();
        phongShader.setVec3("lightPos", lightPosition);
        phongShader.setVec3("viewPos", camera.Position);
        phongShader.setMat4("projection", projection);
        phongShader.setMat4("view", view);
        
        //astéroïde 1
        //positions par rapport au soleil (qui est la source de la lumière) avec un angle de 15°
        float asteroidOrbitAngle = currentFrame * 0.5f;
        float asteroidOrbitRadius = 60.0f;
        float asteroidX = lightPosition.x + asteroidOrbitRadius * cos(asteroidOrbitAngle);
        float asteroidZ = lightPosition.z + asteroidOrbitRadius * sin(asteroidOrbitAngle);
        
        float inclinationAngle = glm::radians(15.0f);
        glm::mat4 inclinationMatrix = glm::rotate(glm::mat4(1.0f), inclinationAngle, glm::vec3(1.0f, 0.0f, 0.0f));
        glm::vec3 asteroidPosition = glm::vec3(
            asteroidOrbitRadius * cos(asteroidOrbitAngle),
            0.0f,
            asteroidOrbitRadius * sin(asteroidOrbitAngle)
        );
        glm::vec4 inclinedPosition = inclinationMatrix * glm::vec4(asteroidPosition, 1.0f);

        glm::mat4 asteroidModel = glm::mat4(1.0f);
        asteroidModel = glm::translate(asteroidModel, glm::vec3(lightPosition.x + inclinedPosition.x, lightPosition.y + inclinedPosition.y, lightPosition.z + inclinedPosition.z));
        asteroidModel = glm::scale(asteroidModel, glm::vec3(0.05f, 0.05f, 0.05f));
        asteroidModel = glm::rotate(asteroidModel, asteroidOrbitAngle * 10.0f, glm::vec3(0.0f, 1.0f, 0.5f));
        phongShader.setMat4("model", asteroidModel);

        //draw
        asteroid1.Draw(phongShader);
        
        //astéroïde 2
        inclinationAngle = glm::radians(11.0f);
        inclinationMatrix = glm::rotate(glm::mat4(1.0f), inclinationAngle, glm::vec3(1.0f, 0.0f, 0.0f));
        glm::vec3 asteroid2Position = glm::vec3(
            asteroidOrbitRadius * cos(asteroidOrbitAngle + glm::radians(10.0f)),
            0.0f,
            asteroidOrbitRadius * sin(asteroidOrbitAngle + glm::radians(10.0f))
        );
        glm::vec4 inclinedPosition2 = inclinationMatrix * glm::vec4(asteroid2Position, 1.0f);
        
        glm::mat4 asteroid2Model = glm::mat4(1.0f);
        asteroid2Model = glm::translate(asteroid2Model, glm::vec3(lightPosition.x + inclinedPosition2.x, lightPosition.y + inclinedPosition2.y, lightPosition.z + inclinedPosition2.z));
        asteroid2Model = glm::scale(asteroid2Model, glm::vec3(0.025f, 0.042f, 0.015f));
        asteroid2Model = glm::rotate(asteroid2Model, asteroidOrbitAngle * 7.0f, glm::vec3(0.5f, 1.0f, 0.0f));
        phongShader.setMat4("model", asteroid2Model);
        
        //draw
        asteroid2.Draw(phongShader);
        
        //astéroïde 3
        inclinationAngle = glm::radians(0.0f);
        inclinationMatrix = glm::rotate(glm::mat4(1.0f), inclinationAngle, glm::vec3(1.0f, 0.0f, 0.0f));
        glm::vec3 asteroid3Position = glm::vec3(
            asteroidOrbitRadius * cos(asteroidOrbitAngle + glm::radians(15.0f)),
            0.0f,
            asteroidOrbitRadius * sin(asteroidOrbitAngle + glm::radians(15.0f))
        );
        glm::vec4 inclinedPosition3 = inclinationMatrix * glm::vec4(asteroid3Position, 1.0f);
        
        glm::mat4 asteroid3Model = glm::mat4(1.0f);
        inclinationMatrix = glm::rotate(glm::mat4(1.0f), inclinationAngle, glm::vec3(1.0f, 0.0f, 0.0f));
        asteroid3Model = glm::translate(asteroid3Model, glm::vec3(lightPosition.x + inclinedPosition3.x, lightPosition.y + inclinedPosition3.y, lightPosition.z + inclinedPosition3.z));
        asteroid3Model = glm::scale(asteroid3Model, glm::vec3(0.08f, 0.08f, 0.08f));
        asteroid3Model = glm::rotate(asteroid3Model, asteroidOrbitAngle * 12.0f, glm::vec3(1.0f, 0.0f, 1.0f));
        phongShader.setMat4("model", asteroid3Model);
        
        //draw
        asteroid3.Draw(phongShader);
        
        ////astéroïde 4
        inclinationAngle = glm::radians(-7.0f);
        glm::vec3 asteroid4Position = glm::vec3(
            asteroidOrbitRadius * cos(asteroidOrbitAngle + glm::radians(20.0f)),
            0.0f,
            asteroidOrbitRadius * sin(asteroidOrbitAngle + glm::radians(20.0f))
        );
        glm::vec4 inclinedPosition4 = inclinationMatrix * glm::vec4(asteroid4Position, 1.0f);
        
        glm::mat4 asteroid4Model = glm::mat4(1.0f);
        asteroid4Model = glm::translate(asteroid4Model, glm::vec3(lightPosition.x + inclinedPosition4.x, lightPosition.y + inclinedPosition4.y, lightPosition.z + inclinedPosition4.z));
        asteroid4Model = glm::scale(asteroid4Model, glm::vec3(0.012f, 0.014f, 0.013f));
        asteroid4Model = glm::rotate(asteroid4Model, asteroidOrbitAngle * 5.0f, glm::vec3(0.3f, 0.7f, 0.2f));
        phongShader.setMat4("model", asteroid4Model);
        
        //draw
        asteroid4.Draw(phongShader);
       
        //======== INTERFACE IMGUI ========
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