#ifndef UBO_H
#define UBO_H

#include <GL/glew.h>
#include <glm/glm.hpp>

struct CameraUBO {
    alignas(16) glm::mat4 projection;
    alignas(16) glm::mat4 view;
    alignas(16) glm::vec3 viewPos;
    alignas(4)  float padding1;
};

struct TransformUBO {
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 normalMatrix;
};

struct LightingUBO {
    alignas(16) glm::vec3 lightPos;
    alignas(4)  float padding1;
    alignas(16) glm::vec3 lightColor;
    alignas(4)  float padding2;
    alignas(16) glm::vec3 ambientColor;
    alignas(4)  float ambientStrength;
    alignas(4)  float specularStrength;
    alignas(4)  float shininess;
    alignas(8)  float padding3[2];
};

enum UBOBindingPoints {
    CAMERA_UBO_BINDING = 0,
    TRANSFORM_UBO_BINDING = 1,
    LIGHTING_UBO_BINDING = 2
};

class UBOManager {
public:
    UBOManager();
    ~UBOManager();
    bool Initialize();
    void Cleanup();
    void UpdateCameraUBO(const glm::mat4& projection, const glm::mat4& view, const glm::vec3& viewPos);
    void UpdateTransformUBO(const glm::mat4& model);
    void UpdateLightingUBO(const glm::vec3& lightPos, const glm::vec3& lightColor, const glm::vec3& ambientColor = glm::vec3(0.1f), float ambientStrength = 0.1f, float specularStrength = 0.5f, float shininess = 32.0f);
    void BindShaderToUBOs(GLuint shaderProgram);
    GLuint GetCameraUBO() const { return cameraUBO; }
    GLuint GetTransformUBO() const { return transformUBO; }
    GLuint GetLightingUBO() const { return lightingUBO; }
private:
    GLuint cameraUBO;
    GLuint transformUBO;
    GLuint lightingUBO;
    bool initialized;
    bool CreateUBO(GLuint& ubo, size_t size, GLuint bindingPoint);
    void UpdateUBO(GLuint ubo, const void* data, size_t size);
};

extern UBOManager* g_uboManager;

#endif
