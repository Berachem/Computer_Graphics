#include "ShaderManager.h"
#include <iostream>

ShaderManager& ShaderManager::getInstance() {
    static ShaderManager instance;
    return instance;
}

bool ShaderManager::Initialize() {
    if (initialized) {
        return true;
    }
    
    std::cout << "Initialisation du ShaderManager..." << std::endl;
    
    try {
        // Charger les shaders d'éclairage
        phongShader = std::make_unique<Shader>("../shaders/phong.vert", "../shaders/phong.frag");
        lambertShader = std::make_unique<Shader>("../shaders/lambert.vert", "../shaders/lambert.frag");
        
        // Charger les autres shaders
        simpleShader = std::make_unique<Shader>("../shaders/simple_color.vert", "../shaders/simple_color.frag");
        texturedShader = std::make_unique<Shader>("../shaders/textured.vert", "../shaders/textured.frag");
        sunShader = std::make_unique<Shader>("../shaders/sun.vert", "../shaders/sun.frag");
        metalShader = std::make_unique<Shader>("../shaders/metal.vert", "../shaders/metal.frag");
        skyboxShader = std::make_unique<Shader>("../shaders/skybox.vert", "../shaders/skybox.frag");
          // Lier les UBOs aux shaders qui en ont besoin
        if (phongShader) phongShader->bindUBOs();
        if (lambertShader) lambertShader->bindUBOs();
        if (texturedShader) texturedShader->bindUBOs();
        if (metalShader) metalShader->bindUBOs();
        if (sunShader) sunShader->bindUBOs();
        if (simpleShader) simpleShader->bindUBOs();
        // Note: skyboxShader n'utilise pas les UBOs de transformation
        
        initialized = true;
        std::cout << "ShaderManager initialisé avec succès" << std::endl;
        std::cout << "- Shaders d'éclairage: Phong, Lambert" << std::endl;
        std::cout << "- Shader actuel: " << (currentLightingShader == LightingShaderType::PHONG ? "Phong" : "Lambert") << std::endl;
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Erreur lors de l'initialisation des shaders: " << e.what() << std::endl;
        return false;
    }
}

void ShaderManager::Cleanup() {
    phongShader.reset();
    lambertShader.reset();
    simpleShader.reset();
    texturedShader.reset();
    sunShader.reset();
    metalShader.reset();
    skyboxShader.reset();
    initialized = false;
    std::cout << "ShaderManager nettoyé" << std::endl;
}

void ShaderManager::SetLightingShaderType(LightingShaderType type) {
    currentLightingShader = type;
    std::cout << "Shader d'éclairage changé vers: " << (type == LightingShaderType::PHONG ? "Phong" : "Lambert") << std::endl;
}

LightingShaderType ShaderManager::GetLightingShaderType() const {
    return currentLightingShader;
}

Shader* ShaderManager::GetCurrentLightingShader() {
    if (!initialized) return nullptr;
    
    switch (currentLightingShader) {
        case LightingShaderType::PHONG:
            return phongShader.get();
        case LightingShaderType::LAMBERT:
            return lambertShader.get();
        default:
            return phongShader.get();
    }
}

Shader* ShaderManager::GetSimpleShader() {
    return initialized ? simpleShader.get() : nullptr;
}

Shader* ShaderManager::GetTexturedShader() {
    return initialized ? texturedShader.get() : nullptr;
}

Shader* ShaderManager::GetSunShader() {
    return initialized ? sunShader.get() : nullptr;
}

Shader* ShaderManager::GetMetalShader() {
    return initialized ? metalShader.get() : nullptr;
}

Shader* ShaderManager::GetSkyboxShader() {
    return initialized ? skyboxShader.get() : nullptr;
}
