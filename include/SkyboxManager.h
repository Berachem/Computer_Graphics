#ifndef SKYBOXMANAGER_H
#define SKYBOXMANAGER_H

#include <vector>
#include <string>
#include <map>

/**
 * @brief Gestionnaire de skyboxes pour faciliter la gestion des différents sets
 * 
 * Cette classe utilitaire permet de définir et récupérer facilement
 * les chemins des différentes skyboxes disponibles.
 */
class SkyboxManager {
public:
    /**
     * @brief Énumération des types de skybox disponibles
     */
    enum class SkyboxType {
        COLORER,    ///< Skybox colorée
        DEFAULT,    ///< Skybox par défaut
        SPACE,       ///< Skybox spatiale
        ZOO          ///< Skybox Zoo
    };

    /**
     * @brief Récupère les chemins des faces d'une skybox
     * @param type Type de skybox demandé
     * @return Vecteur contenant les chemins des 6 faces dans l'ordre OpenGL
     */
    static std::vector<std::string> GetSkyboxFaces(SkyboxType type);

    /**
     * @brief Récupère le nom d'affichage d'un type de skybox
     * @param type Type de skybox
     * @return Nom d'affichage de la skybox
     */
    static std::string GetSkyboxName(SkyboxType type);

private:
    /**
     * @brief Map des chemins de base pour chaque type de skybox
     */
    static const std::map<SkyboxType, std::string> skyboxPaths;
    
    /**
     * @brief Map des noms d'affichage pour chaque type de skybox
     */
    static const std::map<SkyboxType, std::string> skyboxNames;
};

#endif // SKYBOXMANAGER_H
