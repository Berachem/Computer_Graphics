#include "SkyboxManager.h"
#include <iostream>

// Définition des chemins de base pour chaque type de skybox
const std::map<SkyboxManager::SkyboxType, std::string> SkyboxManager::skyboxPaths = {
    {SkyboxType::COLORER, "../skybox/skybox_colorer/"},
    {SkyboxType::DEFAULT, "../skybox/skybox_default/"},
    {SkyboxType::SPACE, "../skybox/skybox_space/"}
};

// Définition des noms d'affichage pour chaque type de skybox
const std::map<SkyboxManager::SkyboxType, std::string> SkyboxManager::skyboxNames = {
    {SkyboxType::COLORER, "Skybox Colorée"},
    {SkyboxType::DEFAULT, "Skybox Par Défaut"},
    {SkyboxType::SPACE, "Skybox Spatiale"}
};

std::vector<std::string> SkyboxManager::GetSkyboxFaces(SkyboxType type) {
    auto it = skyboxPaths.find(type);
    if (it == skyboxPaths.end()) {
        std::cerr << "ERREUR: Type de skybox non reconnu, utilisation de la skybox par défaut" << std::endl;
        it = skyboxPaths.find(SkyboxType::DEFAULT);
    }
    
    const std::string& basePath = it->second;
    
    // Ordre des faces pour OpenGL cubemap:
    // GL_TEXTURE_CUBE_MAP_POSITIVE_X (right)
    // GL_TEXTURE_CUBE_MAP_NEGATIVE_X (left)
    // GL_TEXTURE_CUBE_MAP_POSITIVE_Y (top)
    // GL_TEXTURE_CUBE_MAP_NEGATIVE_Y (bottom)
    // GL_TEXTURE_CUBE_MAP_POSITIVE_Z (front)
    // GL_TEXTURE_CUBE_MAP_NEGATIVE_Z (back)
    return {
        basePath + "right.png",
        basePath + "left.png",
        basePath + "top.png",
        basePath + "bottom.png",
        basePath + "front.png",
        basePath + "back.png"
    };
}

std::string SkyboxManager::GetSkyboxName(SkyboxType type) {
    auto it = skyboxNames.find(type);
    if (it != skyboxNames.end()) {
        return it->second;
    }
    return "Skybox Inconnue";
}
