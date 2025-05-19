#include <GL/glew.h> // Remplacé glad par glew
#include <GLFW/glfw3.h>
#include <iostream>

#include "Shader.h"
#include "Camera.h"
#include "Model.h"
#include "Sphere.h"
#include "TextureLoader.h"
#include <glm/gtc/matrix_transform.hpp>

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

    // Charger les shaders
    Shader simpleShader("../shaders/simple_color.vert", "../shaders/simple_color.frag");
    Shader phongShader("../shaders/phong.vert", "../shaders/phong.frag");
    Shader texturedShader("../shaders/textured.vert", "../shaders/textured.frag");

    // Charger un modèle
    Model myModel("../models/map-bump.obj");

    // Créer une sphère texturée (texture, rayon, secteurs, stacks)
    Sphere moonSphere("../textures/spherical_moon_texture.jpg", 0.5f, 36, 18);

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

        // Activer le shader
        phongShader.use();

        // Matrices
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        phongShader.setMat4("projection", projection);
        phongShader.setMat4("view", view);

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
        texturedShader.setVec3("lightPos", glm::vec3(5.0f, 5.0f, 5.0f));
        texturedShader.setVec3("viewPos", camera.Position);

        // Dessiner la sphère texturée (lune)
        moonSphere.Draw(texturedShader);

        // Revenir au shader phong pour les autres objets
        phongShader.use();
        phongShader.setMat4("model", model);

        // Affichage
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

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
