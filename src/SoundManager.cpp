#include "SoundManager.h"
#include "Sound.h"
#include "AudioSource.h"
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
#define AL_POSITION 1
#define AL_VELOCITY 2
#define AL_ORIENTATION 3
#define AL_GAIN 4
typedef int ALenum;
typedef float ALfloat;
inline ALenum alGetError() { return AL_NO_ERROR; }
inline void alListenerfv(ALenum, const ALfloat*) {}
inline void alListenerf(ALenum, ALfloat) {}
inline const char* alGetString(ALenum) { return "OpenAL non disponible"; }
#define AL_VENDOR 1
#define AL_RENDERER 2
#define AL_VERSION 3
#endif

SoundManager::SoundManager()
    : m_device(nullptr)
    , m_context(nullptr)
    , m_initialized(false)
    , m_masterVolume(1.0f)
{
}

SoundManager::~SoundManager() {
    Shutdown();
}

bool SoundManager::Initialize() {
    if (m_initialized) {
        std::cout << "SoundManager: Déjà initialisé" << std::endl;
        return true;
    }

#ifdef HAVE_OPENAL
    // Ouvrir le périphérique audio par défaut
    m_device = alcOpenDevice(nullptr);
    if (!m_device) {
        std::cerr << "SoundManager: Impossible d'ouvrir le périphérique audio" << std::endl;
        return false;
    }

    // Créer le contexte OpenAL
    m_context = alcCreateContext(m_device, nullptr);
    if (!m_context) {
        std::cerr << "SoundManager: Impossible de créer le contexte OpenAL" << std::endl;
        alcCloseDevice(m_device);
        m_device = nullptr;
        return false;
    }

    // Activer le contexte
    if (!alcMakeContextCurrent(m_context)) {
        std::cerr << "SoundManager: Impossible d'activer le contexte OpenAL" << std::endl;
        alcDestroyContext(m_context);
        alcCloseDevice(m_device);
        m_context = nullptr;
        m_device = nullptr;
        return false;
    }

    // Vérifier les erreurs
    if (!CheckALError("Initialisation OpenAL")) {
        Shutdown();
        return false;
    }

    // Configuration par défaut de l'auditeur
    float listenerPos[] = {0.0f, 0.0f, 0.0f};
    float listenerVel[] = {0.0f, 0.0f, 0.0f};
    float listenerOri[] = {0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f}; // forward, up

    alListenerfv(AL_POSITION, listenerPos);
    alListenerfv(AL_VELOCITY, listenerVel);
    alListenerfv(AL_ORIENTATION, listenerOri);
    alListenerf(AL_GAIN, m_masterVolume);

    if (!CheckALError("Configuration de l'auditeur")) {
        Shutdown();
        return false;
    }

    m_initialized = true;
    std::cout << "SoundManager: Système audio initialisé avec succès" << std::endl;
    
    // Afficher les informations du système audio
    std::cout << "OpenAL Vendor: " << alGetString(AL_VENDOR) << std::endl;
    std::cout << "OpenAL Renderer: " << alGetString(AL_RENDERER) << std::endl;
    std::cout << "OpenAL Version: " << alGetString(AL_VERSION) << std::endl;

    return true;
#else
    // Mode sans OpenAL
    m_device = nullptr;
    m_context = nullptr;
    m_initialized = true;
    std::cout << "SoundManager: Mode sans audio (OpenAL non disponible)" << std::endl;
    return true;
#endif
}

void SoundManager::Shutdown() {
    if (!m_initialized) {
        return;
    }

    // Arrêter tous les sons
    StopAll();

    // Nettoyer les sources audio
    m_audioSources.clear();

    // Nettoyer les sons chargés
    m_sounds.clear();

#ifdef HAVE_OPENAL
    // Désactiver le contexte
    alcMakeContextCurrent(nullptr);

    // Détruire le contexte
    if (m_context) {
        alcDestroyContext(m_context);
        m_context = nullptr;
    }

    // Fermer le périphérique
    if (m_device) {
        alcCloseDevice(m_device);
        m_device = nullptr;
    }
#endif

    m_initialized = false;
    std::cout << "SoundManager: Système audio fermé" << std::endl;
}

std::shared_ptr<Sound> SoundManager::LoadSound(const std::string& filePath, const std::string& soundName) {
    if (!m_initialized) {
        std::cerr << "SoundManager: Système audio non initialisé" << std::endl;
        return nullptr;
    }

#ifndef HAVE_OPENAL
    std::cout << "SoundManager: Chargement simulé de '" << soundName << "' (OpenAL non disponible)" << std::endl;
    auto sound = std::make_shared<Sound>();
    m_sounds[soundName] = sound;
    return sound;
#endif

    // Vérifier si le son est déjà chargé
    auto it = m_sounds.find(soundName);
    if (it != m_sounds.end()) {
        std::cout << "SoundManager: Son '" << soundName << "' déjà chargé" << std::endl;
        return it->second;
    }

    // Créer et charger le nouveau son
    auto sound = std::make_shared<Sound>();
    if (!sound->LoadFromFile(filePath)) {
        std::cerr << "SoundManager: Échec du chargement du son '" << filePath << "'" << std::endl;
        return nullptr;
    }

    // Ajouter à la collection
    m_sounds[soundName] = sound;
    std::cout << "SoundManager: Son '" << soundName << "' chargé avec succès" << std::endl;

    return sound;
}

std::shared_ptr<Sound> SoundManager::GetSound(const std::string& soundName) {
    auto it = m_sounds.find(soundName);
    if (it != m_sounds.end()) {
        return it->second;
    }
    return nullptr;
}

std::shared_ptr<AudioSource> SoundManager::CreateAudioSource() {
    if (!m_initialized) {
        std::cerr << "SoundManager: Système audio non initialisé" << std::endl;
        return nullptr;
    }

#ifndef HAVE_OPENAL
    std::cout << "SoundManager: Création d'une source audio simulée (OpenAL non disponible)" << std::endl;
#endif

    auto audioSource = std::make_shared<AudioSource>();
    m_audioSources.push_back(audioSource);

    return audioSource;
}

void SoundManager::SetListenerPosition(const float position[3], const float forward[3], const float up[3]) {
    if (!m_initialized) return;

    alListenerfv(AL_POSITION, position);
    
    // Orientation: forward + up vectors
    float orientation[6] = {
        forward[0], forward[1], forward[2],
        up[0], up[1], up[2]
    };
    alListenerfv(AL_ORIENTATION, orientation);
    
    CheckALError("SetListenerPosition");
}

void SoundManager::SetMasterVolume(float volume) {
    m_masterVolume = std::max(0.0f, std::min(1.0f, volume));
    
    if (m_initialized) {
        alListenerf(AL_GAIN, m_masterVolume);
        CheckALError("SetMasterVolume");
    }
}

float SoundManager::GetMasterVolume() const {
    return m_masterVolume;
}

void SoundManager::PauseAll() {
    for (auto& audioSource : m_audioSources) {
        if (audioSource && audioSource->IsPlaying()) {
            audioSource->Pause();
        }
    }
}

void SoundManager::ResumeAll() {
    for (auto& audioSource : m_audioSources) {
        if (audioSource && audioSource->IsPaused()) {
            audioSource->Resume();
        }
    }
}

void SoundManager::StopAll() {
    for (auto& audioSource : m_audioSources) {
        if (audioSource) {
            audioSource->Stop();
        }
    }
}

void SoundManager::Update() {
    if (!m_initialized) return;

    // Nettoyer les sources terminées
    CleanupFinishedSources();

    // Mettre à jour les sources actives
    for (auto& audioSource : m_audioSources) {
        if (audioSource) {
            audioSource->Update();
        }
    }
}

bool SoundManager::CheckALError(const std::string& operation) {
    ALenum error = alGetError();
    if (error != AL_NO_ERROR) {
        std::cerr << "SoundManager: Erreur OpenAL lors de '" << operation << "': ";
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

void SoundManager::CleanupFinishedSources() {
    // Supprimer les sources qui ne sont plus utilisées (pointeurs expirés)
    m_audioSources.erase(
        std::remove_if(m_audioSources.begin(), m_audioSources.end(),
            [](const std::shared_ptr<AudioSource>& audioSource) {
                return !audioSource || audioSource.use_count() <= 1;
            }),
        m_audioSources.end()
    );
}
