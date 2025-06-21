#ifndef SHADER_MANAGER_H
#define SHADER_MANAGER_H

#include "Shader.h"
#include <memory>

// Gestionnaire global des shaders
enum class LightingShaderType { PHONG, LAMBERT };

class ShaderManager {
public:
    static ShaderManager& getInstance();
    
    // Initialisation des shaders
    bool Initialize();
    void Cleanup();
    
    // Gestion des shaders d'éclairage
    void SetLightingShaderType(LightingShaderType type);
    LightingShaderType GetLightingShaderType() const;
    Shader* GetCurrentLightingShader();
    
    // Accès aux autres shaders
    Shader* GetSimpleShader();
    Shader* GetTexturedShader();
    Shader* GetSunShader();
    Shader* GetMetalShader();
    Shader* GetSkyboxShader();
    
    // Variables globales pour l'interface
    bool& GetShowShaderSelector() { return showShaderSelector; }
    
private:
    ShaderManager() = default;
    ~ShaderManager() = default;
    ShaderManager(const ShaderManager&) = delete;
    ShaderManager& operator=(const ShaderManager&) = delete;
    
    // Shaders d'éclairage
    std::unique_ptr<Shader> phongShader;
    std::unique_ptr<Shader> lambertShader;
    
    // Autres shaders
    std::unique_ptr<Shader> simpleShader;
    std::unique_ptr<Shader> texturedShader;
    std::unique_ptr<Shader> sunShader;
    std::unique_ptr<Shader> metalShader;
    std::unique_ptr<Shader> skyboxShader;
    
    LightingShaderType currentLightingShader = LightingShaderType::PHONG;
    bool initialized = false;
    
    // Interface utilisateur
    bool showShaderSelector = true;
};

#endif // SHADER_MANAGER_H
