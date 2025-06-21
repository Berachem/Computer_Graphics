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

// Représente un fichier audio chargé en mémoire
class Sound {
public:
    Sound();
    ~Sound();
    bool LoadFromFile(const std::string& filePath);
    bool LoadFromMemory(const void* data, size_t dataSize, int channels, int sampleRate, int bitsPerSample);
    SoundBufferID GetBufferID() const { return m_bufferID; }
    bool IsLoaded() const;
    void Unload();
    float GetDuration() const;
    int GetChannels() const;
    int GetSampleRate() const;
    int GetBitsPerSample() const;
    std::string GetFileName() const;
private:
    SoundBufferID m_bufferID;
    float m_duration;
    int m_channels;
    int m_sampleRate;
    int m_bitsPerSample;
    std::string m_fileName;
    bool LoadWAV(const std::string& filePath);
};

#endif
