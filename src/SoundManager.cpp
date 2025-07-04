#include "SoundManager.h"
#include "Sound.h"
#include "AudioSource.h"
#include <iostream>
#include <algorithm>

// Inclusion conditionnelle de filesystem pour C++17
#if __cplusplus >= 201703L
#include <filesystem>
#endif

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

std::shared_ptr<AudioSource> SoundManager::GetAmbientSource() {
    return m_ambientSource;
}

std::shared_ptr<Sound> SoundManager::GetAmbientSound() {
    return m_ambientSound;
}

void SoundManager::SetupAmbientAudio(const std::string& filePath, const std::string& soundName) {
    if (!m_initialized) return;
    if (!m_ambientSound) {
        m_ambientSound = LoadSound(filePath, soundName);
        m_currentSoundName = soundName;
    }
    if (m_ambientSound && !m_ambientSource) {
        m_ambientSource = CreateAudioSource();
        if (m_ambientSource) {
            m_ambientSource->SetPosition({0.0f, 0.0f, 0.0f});
            m_ambientSource->SetVolume(0.3f);
        }
    }
}

void SoundManager::LoadAllSounds() {
    if (!m_initialized) return;
    
    // Charger automatiquement tous les fichiers .wav du dossier sound/
    std::vector<std::string> wavFiles = {
        "../sound/Zoo.wav",
        "../sound/Zoo.mp3"  // Aussi supporter les mp3 si présents
    };
      // Scanner le dossier sound/ pour tous les fichiers .wav
    // Note: En C++ standard, il faut utiliser std::filesystem (C++17) ou des alternatives
    #if __cplusplus >= 201703L
        // Si C++17 est disponible, utiliser std::filesystem
        try {
            for (const auto& entry : std::filesystem::directory_iterator("../sound/")) {
                if (entry.is_regular_file()) {
                    std::string filepath = entry.path().string();
                    std::string extension = entry.path().extension().string();
                    
                    // Supporter .wav et .mp3
                    if (extension == ".wav" || extension == ".mp3" || extension == ".WAV" || extension == ".MP3") {
                        wavFiles.push_back(filepath);
                    }
                }
            }
        } catch (const std::exception& e) {
            std::cerr << "Erreur lors du scan du dossier sound/: " << e.what() << std::endl;
        }
    #else        // Fallback: ajouter manuellement les fichiers connus
        // Zoo.wav est déjà dans la liste de base
        wavFiles.insert(wavFiles.end(), {
            "../sound/freeze_rael.wav",
            "../sound/je_suis_un_homme.wav", 
            "../sound/spatial_theme.wav"
        });
    #endif
    
    // Charger tous les fichiers trouvés
    for (const std::string& filepath : wavFiles) {
        // Extraire le nom du fichier sans le chemin et l'extension
        size_t lastSlash = filepath.find_last_of("/\\");
        size_t lastDot = filepath.find_last_of(".");
        
        if (lastSlash != std::string::npos && lastDot != std::string::npos && lastDot > lastSlash) {
            std::string filename = filepath.substr(lastSlash + 1, lastDot - lastSlash - 1);
            
            // Charger le son
            if (LoadSound(filepath, filename)) {
                std::cout << "Son chargé: " << filename << " (" << filepath << ")" << std::endl;
            } else {
                std::cout << "Échec du chargement: " << filepath << std::endl;
            }
        }
    }
    
    std::cout << "Total des sons chargés: " << m_sounds.size() << std::endl;
    std::cout << "Sons disponibles:" << std::endl;
    for (const auto& pair : m_sounds) {
        std::cout << "- " << pair.first << std::endl;
    }
}

std::vector<std::string> SoundManager::GetSoundNames() const {
    std::vector<std::string> names;
    for (const auto& pair : m_sounds) {
        names.push_back(pair.first);
    }
    return names;
}

bool SoundManager::SetCurrentAmbientSound(const std::string& soundName) {
    if (!m_initialized || !m_ambientSource) return false;
    
    auto sound = GetSound(soundName);
    if (!sound) {
        std::cerr << "Son non trouvé: " << soundName << std::endl;
        return false;
    }
    
    // Sauvegarder l'état de lecture actuel
    bool wasPlaying = m_ambientSource->IsPlaying();
    bool wasPaused = m_ambientSource->IsPaused();
    
    // Arrêter le son actuel
    if (wasPlaying || wasPaused) {
        m_ambientSource->Stop();
    }
    
    // Changer le son
    m_ambientSound = sound;
    m_currentSoundName = soundName;
    
    // Configurer la source audio avec le nouveau son
    // (on ne joue pas automatiquement, l'utilisateur devra cliquer sur Play)
    
    std::cout << "Son d'ambiance changé vers: " << soundName << std::endl;
    std::cout << "Nouveau son: " << sound->GetFileName() << std::endl;
    
    return true;
}
