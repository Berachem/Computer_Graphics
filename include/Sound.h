#ifndef SOUND_H
#define SOUND_H

#ifdef HAVE_OPENAL
#include <AL/al.h>
typedef ALuint SoundBufferID;
typedef ALenum SoundFormat;
#else
typedef unsigned int SoundBufferID;
typedef int SoundFormat;
#endif

#include <string>
#include <vector>

/**
 * @brief Représente un fichier audio chargé en mémoire
 * 
 * Cette classe encapsule un buffer OpenAL contenant les données audio
 * décodées d'un fichier MP3, WAV ou OGG. Elle peut être utilisée par
 * plusieurs AudioSource simultanément.
 */
class Sound {
public:
    /**
     * @brief Constructeur par défaut
     */
    Sound();
    
    /**
     * @brief Destructeur - Libère le buffer OpenAL
     */
    ~Sound();

    /**
     * @brief Charge un fichier audio
     * @param filePath Chemin vers le fichier audio
     * @return true si le chargement réussit, false sinon
     */
    bool LoadFromFile(const std::string& filePath);

    /**
     * @brief Charge des données audio depuis la mémoire
     * @param data Pointeur vers les données audio PCM
     * @param dataSize Taille des données en octets
     * @param channels Nombre de canaux (1 = mono, 2 = stéréo)
     * @param sampleRate Fréquence d'échantillonnage (ex: 44100 Hz)
     * @param bitsPerSample Bits par échantillon (8 ou 16)
     * @return true si le chargement réussit, false sinon
     */
    bool LoadFromMemory(const void* data, size_t dataSize, int channels, int sampleRate, int bitsPerSample);

    /**
     * @brief Récupère l'ID du buffer OpenAL
     * @return ID du buffer OpenAL, 0 si non chargé
     */
    SoundBufferID GetBufferID() const { return m_bufferID; }

    /**
     * @brief Vérifie si le son est chargé
     * @return true si chargé, false sinon
     */
    bool IsLoaded() const { return m_bufferID != 0; }

    /**
     * @brief Récupère la durée du son en secondes
     * @return Durée en secondes, 0.0f si non chargé
     */
    float GetDuration() const { return m_duration; }

    /**
     * @brief Récupère le nombre de canaux
     * @return Nombre de canaux (1 = mono, 2 = stéréo)
     */
    int GetChannels() const { return m_channels; }

    /**
     * @brief Récupère la fréquence d'échantillonnage
     * @return Fréquence d'échantillonnage en Hz
     */
    int GetSampleRate() const { return m_sampleRate; }

    /**
     * @brief Récupère le nom du fichier
     * @return Nom du fichier source
     */
    const std::string& GetFileName() const { return m_fileName; }

    /**
     * @brief Libère les ressources du son
     */
    void Unload();

private:
    SoundBufferID m_bufferID;   ///< ID du buffer OpenAL
    float m_duration;           ///< Durée du son en secondes
    int m_channels;             ///< Nombre de canaux
    int m_sampleRate;           ///< Fréquence d'échantillonnage
    int m_bitsPerSample;        ///< Bits par échantillon
    std::string m_fileName;     ///< Nom du fichier source

    /**
     * @brief Charge un fichier WAV
     * @param filePath Chemin vers le fichier WAV
     * @return true si le chargement réussit, false sinon
     */
    bool LoadWAV(const std::string& filePath);

    /**
     * @brief Détermine le format OpenAL basé sur les paramètres audio
     * @param channels Nombre de canaux
     * @param bitsPerSample Bits par échantillon
     * @return Format OpenAL (AL_FORMAT_MONO8, AL_FORMAT_MONO16, etc.)
     */
    SoundFormat GetOpenALFormat(int channels, int bitsPerSample);

    /**
     * @brief Calcule la durée basée sur les paramètres audio
     * @param dataSize Taille des données en octets
     * @param channels Nombre de canaux
     * @param sampleRate Fréquence d'échantillonnage
     * @param bitsPerSample Bits par échantillon
     * @return Durée en secondes
     */
    float CalculateDuration(size_t dataSize, int channels, int sampleRate, int bitsPerSample);

    /**
     * @brief Vérifie les erreurs OpenAL
     * @param operation Description de l'opération
     * @return true si aucune erreur, false sinon
     */
    bool CheckALError(const std::string& operation);
};

#endif // SOUND_H
