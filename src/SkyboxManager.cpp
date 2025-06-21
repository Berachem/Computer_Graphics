#include "SkyboxManager.h"
#include <iostream>

const std::map<SkyboxManager::SkyboxType, std::string> SkyboxManager::skyboxPaths = {
    {SkyboxType::COLORER, "../skybox/skybox_colorer/"},
    {SkyboxType::DEFAULT, "../skybox/skybox_default/"},
    {SkyboxType::SPACE, "../skybox/skybox_space/"},
    {SkyboxType::ZOO,   "../skybox/skybox_zoo/"}
};

const std::map<SkyboxManager::SkyboxType, std::string> SkyboxManager::skyboxNames = {
    {SkyboxType::COLORER, "Skybox Colorée"},
    {SkyboxType::DEFAULT, "Skybox Par Défaut"},
    {SkyboxType::SPACE,  "Skybox Spatiale"},
    {SkyboxType::ZOO,    "Skybox Zoo"}
};

std::vector<std::string> SkyboxManager::GetSkyboxFaces(SkyboxType type) {
    auto it = skyboxPaths.find(type);
    if (it == skyboxPaths.end()) it = skyboxPaths.find(SkyboxType::DEFAULT);
    const std::string& basePath = it->second;
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
    if (it != skyboxNames.end()) return it->second;
    return "Skybox Inconnue";
}
