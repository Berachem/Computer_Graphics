#ifndef CAMERA_H
#define CAMERA_H

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Définir les directions de mouvement possibles
enum Camera_Movement
{
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    UP,
    DOWN
};

// Valeurs par défaut
const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 2.5f;
const float SENSITIVITY = 0.1f;
const float ZOOM = 45.0f;

class Camera
{
public:
    // Attributs
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;

    // Angles d'Euler
    float Yaw;
    float Pitch;

    // Options
    float MovementSpeed;
    float MouseSensitivity;
    float Zoom;

    // Constructeur
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f),
           glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
           float yaw = YAW, float pitch = PITCH);

    // Retourner la matrice View
    glm::mat4 GetViewMatrix() const;

    // Gérer l'input clavier
    void ProcessKeyboard(Camera_Movement direction, float deltaTime);

    // (Optionnel) Gérer l'input souris
    void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true);

    // (Optionnel) Gérer la molette de la souris
    void ProcessMouseScroll(float yoffset);

private:
    void updateCameraVectors();
};

#endif
