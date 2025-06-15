#ifndef AUDIOSOURCE_H
#define AUDIOSOURCE_H

#ifdef HAVE_OPENAL
#include <AL/al.h>
typedef ALuint AudioSourceID;
#else
typedef unsigned int AudioSourceID;
#endif

#include <memory>
#include <glm/glm.hpp>

// Forward declaration
class Sound;

/**
 * @brief Source audio 3D dans l'espace
 * 
 * Cette classe représente une source sonore positionnée dans l'espace 3D.
 * Elle peut jouer des sons avec atténuation basée sur la distance,
 * effet Doppler, et positionnement spatial.
 */
class AudioSource {
public:
    /**
     * @brief États possibles de la source audio
     */
    enum class State {
        Stopped,    ///< Arrêtée
        Playing,    ///< En cours de lecture
        Paused      ///< En pause
    };

    /**
     * @brief Constructeur
     */
    AudioSource();
    
    /**
     * @brief Destructeur - Libère la source OpenAL
     */
    ~AudioSource();

    /**
     * @brief Joue un son
     * @param sound Pointeur partagé vers le son à jouer
     * @param loop true pour jouer en boucle, false pour une seule fois
     */
    void Play(std::shared_ptr<Sound> sound, bool loop = false);

    /**
     * @brief Met en pause la lecture
     */
    void Pause();

    /**
     * @brief Reprend la lecture après une pause
     */
    void Resume();

    /**
     * @brief Arrête la lecture
     */
    void Stop();

    /**
     * @brief Définit la position de la source dans l'espace 3D
     * @param position Position (x, y, z)
     */
    void SetPosition(const glm::vec3& position);

    /**
     * @brief Récupère la position de la source
     * @return Position actuelle
     */
    glm::vec3 GetPosition() const { return m_position; }

    /**
     * @brief Définit la vélocité de la source (pour l'effet Doppler)
     * @param velocity Vélocité (x, y, z)
     */
    void SetVelocity(const glm::vec3& velocity);

    /**
     * @brief Récupère la vélocité de la source
     * @return Vélocité actuelle
     */
    glm::vec3 GetVelocity() const { return m_velocity; }

    /**
     * @brief Définit le volume de la source
     * @param volume Volume entre 0.0f et 1.0f
     */
    void SetVolume(float volume);

    /**
     * @brief Récupère le volume de la source
     * @return Volume entre 0.0f et 1.0f
     */
    float GetVolume() const { return m_volume; }

    /**
     * @brief Définit la hauteur (pitch) du son
     * @param pitch Hauteur (1.0f = normal, >1.0f = plus aigu, <1.0f = plus grave)
     */
    void SetPitch(float pitch);

    /**
     * @brief Récupère la hauteur du son
     * @return Hauteur actuelle
     */
    float GetPitch() const { return m_pitch; }

    /**
     * @brief Définit si le son doit jouer en boucle
     * @param loop true pour boucle, false sinon
     */
    void SetLooping(bool loop);

    /**
     * @brief Vérifie si le son joue en boucle
     * @return true si en boucle, false sinon
     */
    bool IsLooping() const { return m_looping; }

    /**
     * @brief Définit la distance de référence pour l'atténuation
     * @param distance Distance de référence
     */
    void SetReferenceDistance(float distance);

    /**
     * @brief Définit la distance maximale d'audibilité
     * @param distance Distance maximale
     */
    void SetMaxDistance(float distance);

    /**
     * @brief Définit le facteur d'atténuation
     * @param rolloff Facteur d'atténuation (1.0f = linéaire)
     */
    void SetRolloffFactor(float rolloff);

    /**
     * @brief Récupère l'état actuel de la source
     * @return État de la source
     */
    State GetState() const;

    /**
     * @brief Vérifie si la source est en cours de lecture
     * @return true si en cours de lecture, false sinon
     */
    bool IsPlaying() const { return GetState() == State::Playing; }

    /**
     * @brief Vérifie si la source est en pause
     * @return true si en pause, false sinon
     */
    bool IsPaused() const { return GetState() == State::Paused; }

    /**
     * @brief Vérifie si la source est arrêtée
     * @return true si arrêtée, false sinon
     */
    bool IsStopped() const { return GetState() == State::Stopped; }

    /**
     * @brief Récupère l'ID de la source OpenAL
     * @return ID de la source OpenAL
     */
    AudioSourceID GetSourceID() const { return m_sourceID; }

    /**
     * @brief Met à jour la source (à appeler chaque frame si nécessaire)
     */
    void Update();

private:
    AudioSourceID m_sourceID;               ///< ID de la source OpenAL
    std::shared_ptr<Sound> m_currentSound;  ///< Son actuellement assigné
    
    // Propriétés de la source
    glm::vec3 m_position;       ///< Position 3D
    glm::vec3 m_velocity;       ///< Vélocité 3D
    float m_volume;             ///< Volume (0.0f - 1.0f)
    float m_pitch;              ///< Hauteur du son
    bool m_looping;             ///< Lecture en boucle
    float m_referenceDistance; ///< Distance de référence
    float m_maxDistance;        ///< Distance maximale
    float m_rolloffFactor;      ///< Facteur d'atténuation

    /**
     * @brief Vérifie les erreurs OpenAL
     * @param operation Description de l'opération
     * @return true si aucune erreur, false sinon
     */
    bool CheckALError(const std::string& operation);
};

#endif // AUDIOSOURCE_H
