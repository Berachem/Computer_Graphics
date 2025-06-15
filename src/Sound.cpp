#include "Sound.h"
#include <iostream>
#include <fstream>
#include <algorithm>
#include <cstring>
#include <cstdint>

#ifndef HAVE_OPENAL
// Définitions factices pour compiler sans OpenAL
#define AL_NO_ERROR 0
#define AL_INVALID_NAME 1
#define AL_INVALID_ENUM 2
#define AL_INVALID_VALUE 3
#define AL_INVALID_OPERATION 4
#define AL_OUT_OF_MEMORY 5
#define AL_FORMAT_MONO8 1
#define AL_FORMAT_MONO16 2
#define AL_FORMAT_STEREO8 3
#define AL_FORMAT_STEREO16 4
typedef int ALenum;
typedef int ALsizei;
inline ALenum alGetError() { return AL_NO_ERROR; }
inline void alGenBuffers(int, unsigned int*) {}
inline void alDeleteBuffers(int, const unsigned int*) {}
inline void alBufferData(unsigned int, int, const void*, int, int) {}
#endif



Sound::Sound()
    : m_bufferID(0)
    , m_duration(0.0f)
    , m_channels(0)
    , m_sampleRate(0)
    , m_bitsPerSample(0)
{
}

Sound::~Sound() {
    Unload();
}

bool Sound::LoadFromFile(const std::string& filePath) {
    // Extraire l'extension du fichier
    size_t dotPos = filePath.find_last_of('.');
    if (dotPos == std::string::npos) {
        std::cerr << "Sound: Impossible de déterminer le format du fichier '" << filePath << "'" << std::endl;
        return false;
    }

    std::string extension = filePath.substr(dotPos + 1);
    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);

    m_fileName = filePath.substr(filePath.find_last_of("/\\") + 1);

    // Seul le format WAV est supporté
    if (extension == "wav") {
        return LoadWAV(filePath);
    } else {
        std::cerr << "Sound: Seul le format WAV est supporté. Format trouvé: '" << extension << "'" << std::endl;
        return false;
    }
}

bool Sound::LoadFromMemory(const void* data, size_t dataSize, int channels, int sampleRate, int bitsPerSample) {
    if (!data || dataSize == 0 || channels <= 0 || sampleRate <= 0 || (bitsPerSample != 8 && bitsPerSample != 16)) {
        std::cerr << "Sound: Paramètres invalides pour LoadFromMemory" << std::endl;
        return false;
    }

    // Libérer le buffer existant si nécessaire
    Unload();

#ifndef HAVE_OPENAL
    // Mode simulation sans OpenAL
    m_bufferID = 1; // ID factice
    m_channels = channels;
    m_sampleRate = sampleRate;
    m_bitsPerSample = bitsPerSample;
    m_duration = CalculateDuration(dataSize, channels, sampleRate, bitsPerSample);
    m_fileName = "Memory Buffer (Simulé)";
    std::cout << "Sound: Chargement simulé depuis la mémoire - " << m_channels << " canaux, "
              << m_sampleRate << " Hz, " << m_duration << "s" << std::endl;
    return true;
#endif

    // Générer un nouveau buffer OpenAL
    alGenBuffers(1, &m_bufferID);
    if (!CheckALError("Génération du buffer")) {
        return false;
    }

    // Déterminer le format OpenAL
    SoundFormat format = GetOpenALFormat(channels, bitsPerSample);
    if (format == 0) {
        std::cerr << "Sound: Format audio non supporté (" << channels << " canaux, " << bitsPerSample << " bits)" << std::endl;
        Unload();
        return false;
    }

    // Charger les données dans le buffer
    alBufferData(m_bufferID, format, data, static_cast<ALsizei>(dataSize), sampleRate);
    if (!CheckALError("Chargement des données dans le buffer")) {
        Unload();
        return false;
    }

    // Sauvegarder les propriétés
    m_channels = channels;
    m_sampleRate = sampleRate;
    m_bitsPerSample = bitsPerSample;
    m_duration = CalculateDuration(dataSize, channels, sampleRate, bitsPerSample);
    m_fileName = "Memory Buffer";

    std::cout << "Sound: Chargé depuis la mémoire - " << m_channels << " canaux, " 
              << m_sampleRate << " Hz, " << m_duration << "s" << std::endl;

    return true;
}

void Sound::Unload() {
    if (m_bufferID != 0) {
#ifdef HAVE_OPENAL
        alDeleteBuffers(1, &m_bufferID);
        CheckALError("Suppression du buffer");
#endif
        m_bufferID = 0;
    }

    m_duration = 0.0f;
    m_channels = 0;
    m_sampleRate = 0;
    m_bitsPerSample = 0;
    m_fileName.clear();
}



bool Sound::LoadWAV(const std::string& filePath) {
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Sound: Impossible d'ouvrir le fichier '" << filePath << "'" << std::endl;
        return false;
    }

    char riff[4];
    file.read(riff, 4);
    if (std::memcmp(riff, "RIFF", 4) != 0) {
        std::cerr << "Sound: Signature RIFF absente" << std::endl;
        return false;
    }

    file.ignore(4); // Taille totale

    char wave[4];
    file.read(wave, 4);
    if (std::memcmp(wave, "WAVE", 4) != 0) {
        std::cerr << "Sound: Signature WAVE absente" << std::endl;
        return false;
    }

    uint16_t audioFormat = 0;
    uint16_t numChannels = 0;
    uint32_t sampleRate = 0;
    uint16_t bitsPerSample = 0;
    std::vector<char> audioData;

    while (!file.eof()) {
        char chunkId[4];
        file.read(chunkId, 4);

        uint32_t chunkSize = 0;
        file.read(reinterpret_cast<char*>(&chunkSize), 4);

        if (file.gcount() < 4) break;

        if (std::memcmp(chunkId, "fmt ", 4) == 0) {
            file.read(reinterpret_cast<char*>(&audioFormat), 2);
            file.read(reinterpret_cast<char*>(&numChannels), 2);
            file.read(reinterpret_cast<char*>(&sampleRate), 4);
            file.ignore(6); // ByteRate + BlockAlign
            file.read(reinterpret_cast<char*>(&bitsPerSample), 2);
            file.ignore(chunkSize - 16); // ignorer le reste du chunk fmt
        }
        else if (std::memcmp(chunkId, "data", 4) == 0) {
            audioData.resize(chunkSize);
            file.read(audioData.data(), chunkSize);
            break; // on a trouvé les données audio
        }
        else {
            // ignorer les chunks inconnus
            file.ignore(chunkSize);
        }
    }

    if (audioData.empty()) {
        std::cerr << "Sound: Chunk 'data' non trouvé ou vide" << std::endl;
        return false;
    }

    // Charger dans OpenAL
    return LoadFromMemory(audioData.data(), audioData.size(), numChannels, sampleRate, bitsPerSample);
}

SoundFormat Sound::GetOpenALFormat(int channels, int bitsPerSample) {
#ifdef HAVE_OPENAL
    if (channels == 1) {
        if (bitsPerSample == 8) return AL_FORMAT_MONO8;
        if (bitsPerSample == 16) return AL_FORMAT_MONO16;
    } else if (channels == 2) {
        if (bitsPerSample == 8) return AL_FORMAT_STEREO8;
        if (bitsPerSample == 16) return AL_FORMAT_STEREO16;
    }
    return 0; // Format non supporté
#else
    // Mode simulation
    return 1; // Format factice
#endif
}

float Sound::CalculateDuration(size_t dataSize, int channels, int sampleRate, int bitsPerSample) {
    if (channels <= 0 || sampleRate <= 0 || bitsPerSample <= 0) {
        return 0.0f;
    }

    size_t bytesPerSample = (bitsPerSample / 8) * channels;
    size_t totalSamples = dataSize / bytesPerSample;
    
    return static_cast<float>(totalSamples) / static_cast<float>(sampleRate);
}

bool Sound::CheckALError(const std::string& operation) {
    ALenum error = alGetError();
    if (error != AL_NO_ERROR) {
        std::cerr << "Sound: Erreur OpenAL lors de '" << operation << "': ";
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
