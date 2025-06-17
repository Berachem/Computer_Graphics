#ifndef SOUNDMANAGER_H
#define SOUNDMANAGER_H

#ifdef HAVE_OPENAL
#include <AL/al.h>
#include <AL/alc.h>
#endif

#include <vector>
#include <memory>
#include <string>
#include <unordered_map>

// Forward declarations
class Sound;
class AudioSource;

/**
 * @brief Gestionnaire principal du système audio
 * 
 * Cette classe gère l'initialisation d'OpenAL, les sources audio globales,
 * et fournit une interface centralisée pour la gestion des sons dans la scène 3D.
 */
class SoundManager {
public:
    /**
     * @brief Constructeur - Initialise le système audio OpenAL
     */
    SoundManager();
    
    /**
     * @brief Destructeur - Nettoie les ressources OpenAL
     */
    ~SoundManager();

    /**
     * @brief Initialise le système audio
     * @return true si l'initialisation réussit, false sinon
     */
    bool Initialize();

    /**
     * @brief Nettoie et ferme le système audio
     */
    void Shutdown();

    /**
     * @brief Charge un fichier audio et retourne un pointeur vers l'objet Sound
     * @param filePath Chemin vers le fichier audio (MP3, WAV, OGG)
     * @param soundName Nom unique pour identifier le son
     * @return Pointeur partagé vers l'objet Sound, nullptr si échec
     */
    std::shared_ptr<Sound> LoadSound(const std::string& filePath, const std::string& soundName);

    /**
     * @brief Récupère un son chargé par son nom
     * @param soundName Nom du son
     * @return Pointeur partagé vers l'objet Sound, nullptr si non trouvé
     */
    std::shared_ptr<Sound> GetSound(const std::string& soundName);

    /**
     * @brief Crée une nouvelle source audio 3D
     * @return Pointeur partagé vers l'objet AudioSource
     */
    std::shared_ptr<AudioSource> CreateAudioSource();

    /**
     * @brief Met à jour la position de l'auditeur (caméra)
     * @param position Position de l'auditeur dans l'espace 3D
     * @param forward Vecteur direction avant de l'auditeur
     * @param up Vecteur direction haut de l'auditeur
     */
    void SetListenerPosition(const float position[3], const float forward[3], const float up[3]);

    /**
     * @brief Définit le volume principal du système audio
     * @param volume Volume entre 0.0f et 1.0f
     */
    void SetMasterVolume(float volume);

    /**
     * @brief Récupère le volume principal
     * @return Volume principal entre 0.0f et 1.0f
     */
    float GetMasterVolume() const;

    /**
     * @brief Met en pause tous les sons en cours de lecture
     */
    void PauseAll();

    /**
     * @brief Reprend tous les sons en pause
     */
    void ResumeAll();

    /**
     * @brief Arrête tous les sons en cours de lecture
     */
    void StopAll();

    /**
     * @brief Vérifie si le système audio est initialisé
     * @return true si initialisé, false sinon
     */
    bool IsInitialized() const { return m_initialized; }

    /**
     * @brief Met à jour le système audio (à appeler chaque frame)
     * Nettoie les sources audio terminées et met à jour les états
     */
    void Update();

    /**
     * @brief Accès à la source audio d'ambiance mutualisée
     */
    std::shared_ptr<AudioSource> GetAmbientSource();

    /**
     * @brief Accès au son d'ambiance mutualisé
     */
    std::shared_ptr<Sound> GetAmbientSound();

    /**
     * @brief Initialise la source et le son d'ambiance (à appeler après Initialize)
     */
    void SetupAmbientAudio(const std::string& filePath = "../sound/Zoo.wav", const std::string& soundName = "zoo_ambient");

private:
    // Contexte OpenAL
#ifdef HAVE_OPENAL
    ALCdevice* m_device;
    ALCcontext* m_context;
#else
    void* m_device;
    void* m_context;
#endif
    
    // État du système
    bool m_initialized;
    float m_masterVolume;
    
    // Collections des ressources audio
    std::unordered_map<std::string, std::shared_ptr<Sound>> m_sounds;
    std::vector<std::shared_ptr<AudioSource>> m_audioSources;

    // === Ambiance mutualisée ===
    std::shared_ptr<Sound> m_ambientSound;
    std::shared_ptr<AudioSource> m_ambientSource;
    
    /**
     * @brief Vérifie les erreurs OpenAL et les affiche
     * @param operation Description de l'opération en cours
     * @return true si aucune erreur, false sinon
     */
    bool CheckALError(const std::string& operation);
    
    /**
     * @brief Nettoie les sources audio terminées
     */
    void CleanupFinishedSources();
};

#endif // SOUNDMANAGER_H
