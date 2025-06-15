#include "AudioSource.h"
#include "Sound.h"
#include <iostream>
#include <algorithm>

#ifndef HAVE_OPENAL
// Définitions factices pour compiler sans OpenAL
#define AL_NO_ERROR 0
#define AL_INVALID_NAME 1
#define AL_INVALID_ENUM 2
#define AL_INVALID_VALUE 3
#define AL_INVALID_OPERATION 4
#define AL_OUT_OF_MEMORY 5
#define AL_GAIN 1
#define AL_PITCH 2
#define AL_LOOPING 3
#define AL_REFERENCE_DISTANCE 4
#define AL_MAX_DISTANCE 5
#define AL_ROLLOFF_FACTOR 6
#define AL_POSITION 7
#define AL_VELOCITY 8
#define AL_BUFFER 9
#define AL_SOURCE_STATE 10
#define AL_PLAYING 11
#define AL_PAUSED 12
#define AL_STOPPED 13
#define AL_INITIAL 14
#define AL_TRUE 1
#define AL_FALSE 0
typedef int ALenum;
typedef int ALint;
typedef float ALfloat;
inline ALenum alGetError() { return AL_NO_ERROR; }
inline void alGenSources(int, unsigned int*) {}
inline void alDeleteSources(int, const unsigned int*) {}
inline void alSourcef(unsigned int, int, float) {}
inline void alSourcei(unsigned int, int, int) {}
inline void alSource3f(unsigned int, int, float, float, float) {}
inline void alSourcePlay(unsigned int) {}
inline void alSourcePause(unsigned int) {}
inline void alSourceStop(unsigned int) {}
inline void alGetSourcei(unsigned int, int, int* value) { if(value) *value = AL_STOPPED; }
#endif

AudioSource::AudioSource()
    : m_sourceID(0)
    , m_position(0.0f, 0.0f, 0.0f)
    , m_velocity(0.0f, 0.0f, 0.0f)
    , m_volume(1.0f)
    , m_pitch(1.0f)
    , m_looping(false)
    , m_referenceDistance(1.0f)
    , m_maxDistance(100.0f)
    , m_rolloffFactor(1.0f)
{
#ifdef HAVE_OPENAL
    // Générer une source OpenAL
    alGenSources(1, &m_sourceID);
    if (!CheckALError("Génération de la source audio")) {
        m_sourceID = 0;
        return;
    }
#else
    // Mode simulation
    m_sourceID = 1; // ID factice
#endif

    // Configuration par défaut de la source
    alSourcef(m_sourceID, AL_GAIN, m_volume);
    alSourcef(m_sourceID, AL_PITCH, m_pitch);
    alSourcei(m_sourceID, AL_LOOPING, m_looping ? AL_TRUE : AL_FALSE);
    alSourcef(m_sourceID, AL_REFERENCE_DISTANCE, m_referenceDistance);
    alSourcef(m_sourceID, AL_MAX_DISTANCE, m_maxDistance);
    alSourcef(m_sourceID, AL_ROLLOFF_FACTOR, m_rolloffFactor);
    
    // Position et vélocité initiales
    alSource3f(m_sourceID, AL_POSITION, m_position.x, m_position.y, m_position.z);
    alSource3f(m_sourceID, AL_VELOCITY, m_velocity.x, m_velocity.y, m_velocity.z);

    CheckALError("Configuration initiale de la source");
}

AudioSource::~AudioSource() {
    if (m_sourceID != 0) {
#ifdef HAVE_OPENAL
        // Arrêter la source avant de la supprimer
        alSourceStop(m_sourceID);
        alDeleteSources(1, &m_sourceID);
        CheckALError("Suppression de la source audio");
#endif
    }
}

void AudioSource::Play(std::shared_ptr<Sound> sound, bool loop) {
    if (!sound || !sound->IsLoaded() || m_sourceID == 0) {
        std::cerr << "AudioSource: Son invalide ou source non initialisée" << std::endl;
        return;
    }

#ifndef HAVE_OPENAL
    std::cout << "AudioSource: Lecture simulée de '" << sound->GetFileName() << "'" << std::endl;
    m_currentSound = sound;
    return;
#endif

    // Arrêter la lecture actuelle si nécessaire
    Stop();

    // Assigner le nouveau son
    m_currentSound = sound;
    alSourcei(m_sourceID, AL_BUFFER, static_cast<ALint>(sound->GetBufferID()));

    // Configurer la boucle
    SetLooping(loop);

    // Démarrer la lecture
    alSourcePlay(m_sourceID);

    if (CheckALError("Lecture du son")) {
        std::cout << "AudioSource: Lecture de '" << sound->GetFileName() << "'" << std::endl;
    }
}

void AudioSource::Pause() {
    if (m_sourceID != 0 && IsPlaying()) {
        alSourcePause(m_sourceID);
        CheckALError("Pause de la source");
    }
}

void AudioSource::Resume() {
    if (m_sourceID != 0 && IsPaused()) {
        alSourcePlay(m_sourceID);
        CheckALError("Reprise de la source");
    }
}

void AudioSource::Stop() {
    if (m_sourceID != 0) {
        alSourceStop(m_sourceID);
        CheckALError("Arrêt de la source");
    }
}

void AudioSource::SetPosition(const glm::vec3& position) {
    m_position = position;
    if (m_sourceID != 0) {
        alSource3f(m_sourceID, AL_POSITION, position.x, position.y, position.z);
        CheckALError("SetPosition");
    }
}

void AudioSource::SetVelocity(const glm::vec3& velocity) {
    m_velocity = velocity;
    if (m_sourceID != 0) {
        alSource3f(m_sourceID, AL_VELOCITY, velocity.x, velocity.y, velocity.z);
        CheckALError("SetVelocity");
    }
}

void AudioSource::SetVolume(float volume) {
    m_volume = std::max(0.0f, std::min(1.0f, volume));
    if (m_sourceID != 0) {
        alSourcef(m_sourceID, AL_GAIN, m_volume);
        CheckALError("SetVolume");
    }
}

void AudioSource::SetPitch(float pitch) {
    m_pitch = std::max(0.1f, std::min(2.0f, pitch)); // Limiter entre 0.1 et 2.0
    if (m_sourceID != 0) {
        alSourcef(m_sourceID, AL_PITCH, m_pitch);
        CheckALError("SetPitch");
    }
}

void AudioSource::SetLooping(bool loop) {
    m_looping = loop;
    if (m_sourceID != 0) {
        alSourcei(m_sourceID, AL_LOOPING, loop ? AL_TRUE : AL_FALSE);
        CheckALError("SetLooping");
    }
}

void AudioSource::SetReferenceDistance(float distance) {
    m_referenceDistance = std::max(0.1f, distance);
    if (m_sourceID != 0) {
        alSourcef(m_sourceID, AL_REFERENCE_DISTANCE, m_referenceDistance);
        CheckALError("SetReferenceDistance");
    }
}

void AudioSource::SetMaxDistance(float distance) {
    m_maxDistance = std::max(1.0f, distance);
    if (m_sourceID != 0) {
        alSourcef(m_sourceID, AL_MAX_DISTANCE, m_maxDistance);
        CheckALError("SetMaxDistance");
    }
}

void AudioSource::SetRolloffFactor(float rolloff) {
    m_rolloffFactor = std::max(0.0f, rolloff);
    if (m_sourceID != 0) {
        alSourcef(m_sourceID, AL_ROLLOFF_FACTOR, m_rolloffFactor);
        CheckALError("SetRolloffFactor");
    }
}

AudioSource::State AudioSource::GetState() const {
    if (m_sourceID == 0) {
        return State::Stopped;
    }

#ifndef HAVE_OPENAL
    // Mode simulation - toujours arrêté
    return State::Stopped;
#endif

    ALint state;
    alGetSourcei(m_sourceID, AL_SOURCE_STATE, &state);

    switch (state) {
        case AL_PLAYING:
            return State::Playing;
        case AL_PAUSED:
            return State::Paused;
        case AL_STOPPED:
        case AL_INITIAL:
        default:
            return State::Stopped;
    }
}

void AudioSource::Update() {
    // Cette méthode peut être utilisée pour des mises à jour spécifiques
    // comme la gestion des effets, la synchronisation, etc.
    
    // Pour l'instant, on vérifie juste si la source est toujours valide
    if (m_sourceID != 0) {
        // Vérifier si la source a terminé de jouer (pour les sons non-bouclés)
        if (!m_looping && GetState() == State::Stopped && m_currentSound) {
            // Le son a terminé, on peut nettoyer la référence
            // (optionnel, selon les besoins de l'application)
        }
    }
}

bool AudioSource::CheckALError(const std::string& operation) {
    ALenum error = alGetError();
    if (error != AL_NO_ERROR) {
        std::cerr << "AudioSource: Erreur OpenAL lors de '" << operation << "': ";
        switch (error) {
            case AL_INVALID_NAME:
                std::cerr << "AL_INVALID_NAME";
                break;
            case AL_INVALID_ENUM:
                std::cerr << "AL_INVALID_ENUM";
                break;
            case AL_INVALID_VALUE:
                std::cerr << "AL_INVALID_VALUE";
                break;
            case AL_INVALID_OPERATION:
                std::cerr << "AL_INVALID_OPERATION";
                break;
            case AL_OUT_OF_MEMORY:
                std::cerr << "AL_OUT_OF_MEMORY";
                break;
            default:
                std::cerr << "Erreur inconnue (" << error << ")";
                break;
        }
        std::cerr << std::endl;
        return false;
    }
    return true;
}
