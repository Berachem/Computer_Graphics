#ifndef UBO_H
#define UBO_H

#include <GL/glew.h>
#include <glm/glm.hpp>

// Structure pour les données de caméra (matrices de transformation)
struct CameraUBO {
    alignas(16) glm::mat4 projection;
    alignas(16) glm::mat4 view;
    alignas(16) glm::vec3 viewPos;
    alignas(4)  float padding1; // Alignement 16 bytes
};

// Structure pour les données de transformation
struct TransformUBO {
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 normalMatrix; // mat3 étendue en mat4 pour l'alignement
};

// Structure pour les données d'éclairage
struct LightingUBO {
    alignas(16) glm::vec3 lightPos;
    alignas(4)  float padding1;
    alignas(16) glm::vec3 lightColor;
    alignas(4)  float padding2;
    alignas(16) glm::vec3 ambientColor;
    alignas(4)  float ambientStrength;
    alignas(4)  float specularStrength;
    alignas(4)  float shininess;
    alignas(8)  float padding3[2]; // Compléter l'alignement
};

// Enum pour les points de liaison des UBOs
enum UBOBindingPoints {
    CAMERA_UBO_BINDING = 0,
    TRANSFORM_UBO_BINDING = 1,
    LIGHTING_UBO_BINDING = 2
};

// Classe pour gérer les UBOs
class UBOManager {
public:
    UBOManager();
    ~UBOManager();
    
    // Initialisation des UBOs
    bool Initialize();
    void Cleanup();
    
    // Mise à jour des données
    void UpdateCameraUBO(const glm::mat4& projection, const glm::mat4& view, const glm::vec3& viewPos);
    void UpdateTransformUBO(const glm::mat4& model);
    void UpdateLightingUBO(const glm::vec3& lightPos, const glm::vec3& lightColor, 
                          const glm::vec3& ambientColor = glm::vec3(0.1f), 
                          float ambientStrength = 0.1f, 
                          float specularStrength = 0.5f, 
                          float shininess = 32.0f);
    
    // Liaison des shaders aux UBOs
    void BindShaderToUBOs(GLuint shaderProgram);
    
    // Getters pour les IDs des UBOs
    GLuint GetCameraUBO() const { return cameraUBO; }
    GLuint GetTransformUBO() const { return transformUBO; }
    GLuint GetLightingUBO() const { return lightingUBO; }
    
private:
    GLuint cameraUBO;
    GLuint transformUBO;
    GLuint lightingUBO;
    
    bool initialized;
    
    // Méthodes utilitaires privées
    bool CreateUBO(GLuint& ubo, size_t size, GLuint bindingPoint);
    void UpdateUBO(GLuint ubo, const void* data, size_t size);
};

// Instance globale du gestionnaire UBO
extern UBOManager* g_uboManager;

#endif // UBO_H
