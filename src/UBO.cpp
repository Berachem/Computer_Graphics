#include "UBO.h"
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>

UBOManager* g_uboManager = nullptr;

UBOManager::UBOManager() : cameraUBO(0), transformUBO(0), lightingUBO(0), initialized(false) {}
UBOManager::~UBOManager() { Cleanup(); }

bool UBOManager::Initialize() {
    if (initialized) return true;
    
    // Créer les UBOs
    if (!CreateUBO(cameraUBO, sizeof(CameraUBO), CAMERA_UBO_BINDING)) return false;
    if (!CreateUBO(transformUBO, sizeof(TransformUBO), TRANSFORM_UBO_BINDING)) return false;
    if (!CreateUBO(lightingUBO, sizeof(LightingUBO), LIGHTING_UBO_BINDING)) return false;
    
    initialized = true;
    
    return true;
}

void UBOManager::Cleanup() {
    if (cameraUBO != 0) { glDeleteBuffers(1, &cameraUBO); cameraUBO = 0; }
    if (transformUBO != 0) { glDeleteBuffers(1, &transformUBO); transformUBO = 0; }
    if (lightingUBO != 0) { glDeleteBuffers(1, &lightingUBO); lightingUBO = 0; }
    initialized = false;
}

bool UBOManager::CreateUBO(GLuint& ubo, size_t size, GLuint bindingPoint) {
    glGenBuffers(1, &ubo);
    if (ubo == 0) return false;
    
    glBindBuffer(GL_UNIFORM_BUFFER, ubo);
    glBufferData(GL_UNIFORM_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, bindingPoint, ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    
    // Vérifier les erreurs OpenGL
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) return false;
    
    return true;
}

void UBOManager::UpdateUBO(GLuint ubo, const void* data, size_t size) {
    glBindBuffer(GL_UNIFORM_BUFFER, ubo);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, size, data);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void UBOManager::UpdateCameraUBO(const glm::mat4& projection, const glm::mat4& view, const glm::vec3& viewPos) {
    if (!initialized) return;
    
    CameraUBO cameraData;
    cameraData.projection = projection;
    cameraData.view = view;
    cameraData.viewPos = viewPos;
    cameraData.padding1 = 0.0f;
    
    UpdateUBO(cameraUBO, &cameraData, sizeof(CameraUBO));
}

void UBOManager::UpdateTransformUBO(const glm::mat4& model) {
    if (!initialized) return;
    
    TransformUBO transformData;
    transformData.model = model;
    
    // Calculer la matrice normale (inverse transposée du modèle)
    glm::mat3 normalMat = glm::mat3(glm::transpose(glm::inverse(model)));
    // Convertir en mat4 pour l'alignement UBO
    transformData.normalMatrix = glm::mat4(
        glm::vec4(normalMat[0], 0.0f),
        glm::vec4(normalMat[1], 0.0f),
        glm::vec4(normalMat[2], 0.0f),
        glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)
    );
    
    UpdateUBO(transformUBO, &transformData, sizeof(TransformUBO));
}

void UBOManager::UpdateLightingUBO(const glm::vec3& lightPos, const glm::vec3& lightColor, 
                                  const glm::vec3& ambientColor, float ambientStrength, 
                                  float specularStrength, float shininess) {
    if (!initialized) return;
    
    LightingUBO lightingData;
    lightingData.lightPos = lightPos;
    lightingData.padding1 = 0.0f;
    lightingData.lightColor = lightColor;
    lightingData.padding2 = 0.0f;
    lightingData.ambientColor = ambientColor;
    lightingData.ambientStrength = ambientStrength;
    lightingData.specularStrength = specularStrength;
    lightingData.shininess = shininess;
    lightingData.padding3[0] = 0.0f;
    lightingData.padding3[1] = 0.0f;
    
    UpdateUBO(lightingUBO, &lightingData, sizeof(LightingUBO));
}

void UBOManager::BindShaderToUBOs(GLuint shaderProgram) {
    if (!initialized) return;
    
    // Lier les blocs uniformes aux points de liaison
    GLuint cameraBlockIndex = glGetUniformBlockIndex(shaderProgram, "CameraUBO");
    if (cameraBlockIndex != GL_INVALID_INDEX) glUniformBlockBinding(shaderProgram, cameraBlockIndex, CAMERA_UBO_BINDING);
    
    GLuint transformBlockIndex = glGetUniformBlockIndex(shaderProgram, "TransformUBO");
    if (transformBlockIndex != GL_INVALID_INDEX) glUniformBlockBinding(shaderProgram, transformBlockIndex, TRANSFORM_UBO_BINDING);
    
    GLuint lightingBlockIndex = glGetUniformBlockIndex(shaderProgram, "LightingUBO");
    if (lightingBlockIndex != GL_INVALID_INDEX) glUniformBlockBinding(shaderProgram, lightingBlockIndex, LIGHTING_UBO_BINDING);
}
