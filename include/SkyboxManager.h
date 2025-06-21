#ifndef SKYBOXMANAGER_H
#define SKYBOXMANAGER_H

#include <vector>
#include <string>
#include <map>

// Gestionnaire de skyboxes
class SkyboxManager {
public:
    enum class SkyboxType { COLORER, DEFAULT, SPACE, ZOO };
    static std::vector<std::string> GetSkyboxFaces(SkyboxType type);
    static std::string GetSkyboxName(SkyboxType type);
private:
    static const std::map<SkyboxType, std::string> skyboxPaths;
    static const std::map<SkyboxType, std::string> skyboxNames;
};

#endif // SKYBOXMANAGER_H
