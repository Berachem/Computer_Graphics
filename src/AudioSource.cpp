#include "AudioSource.h"
#include "Sound.h"
#include <algorithm>

#ifndef HAVE_OPENAL
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
    : m_sourceID(0), m_position(0.0f), m_velocity(0.0f), m_volume(1.0f), m_pitch(1.0f), m_looping(false), m_referenceDistance(1.0f), m_maxDistance(100.0f), m_rolloffFactor(1.0f) {
#ifdef HAVE_OPENAL
    alGenSources(1, &m_sourceID);
    if (!CheckALError("AudioSource creation")) {
        m_sourceID = 0;
    }
#endif
}

AudioSource::~AudioSource() {
    Cleanup();
}

void AudioSource::Play(std::shared_ptr<Sound> sound, bool loop) {
    m_currentSound = sound;
    m_looping = loop;
#ifdef HAVE_OPENAL
    if (m_sourceID && sound) {
        alSourcei(m_sourceID, AL_BUFFER, sound->GetBufferID());
        alSourcei(m_sourceID, AL_LOOPING, loop ? AL_TRUE : AL_FALSE);
        alSourcePlay(m_sourceID);
        m_state = State::Playing;
    }
#endif
}

void AudioSource::Pause() {
#ifdef HAVE_OPENAL
    if (m_sourceID) {
        alSourcePause(m_sourceID);
        m_state = State::Paused;
    }
#endif
}

void AudioSource::Resume() {
#ifdef HAVE_OPENAL
    if (m_sourceID) {
        alSourcePlay(m_sourceID);
        m_state = State::Playing;
    }
#endif
}

void AudioSource::Stop() {
#ifdef HAVE_OPENAL
    if (m_sourceID) {
        alSourceStop(m_sourceID);
        m_state = State::Stopped;
    }
#endif
}

void AudioSource::SetPosition(const glm::vec3& position) {
    m_position = position;
#ifdef HAVE_OPENAL
    if (m_sourceID) {
        alSource3f(m_sourceID, AL_POSITION, position.x, position.y, position.z);
    }
#endif
}

void AudioSource::SetVelocity(const glm::vec3& velocity) {
    m_velocity = velocity;
#ifdef HAVE_OPENAL
    if (m_sourceID) {
        alSource3f(m_sourceID, AL_VELOCITY, velocity.x, velocity.y, velocity.z);
    }
#endif
}

void AudioSource::SetVolume(float volume) {
    m_volume = volume;
#ifdef HAVE_OPENAL
    if (m_sourceID) {
        alSourcef(m_sourceID, AL_GAIN, volume);
    }
#endif
}

void AudioSource::SetPitch(float pitch) {
    m_pitch = pitch;
#ifdef HAVE_OPENAL
    if (m_sourceID) {
        alSourcef(m_sourceID, AL_PITCH, pitch);
    }
#endif
}

void AudioSource::SetLooping(bool loop) {
    m_looping = loop;
#ifdef HAVE_OPENAL
    if (m_sourceID) {
        alSourcei(m_sourceID, AL_LOOPING, loop ? AL_TRUE : AL_FALSE);
    }
#endif
}

void AudioSource::SetReferenceDistance(float distance) {
    m_referenceDistance = distance;
#ifdef HAVE_OPENAL
    if (m_sourceID) {
        alSourcef(m_sourceID, AL_REFERENCE_DISTANCE, distance);
    }
#endif
}

void AudioSource::SetMaxDistance(float distance) {
    m_maxDistance = distance;
#ifdef HAVE_OPENAL
    if (m_sourceID) {
        alSourcef(m_sourceID, AL_MAX_DISTANCE, distance);
    }
#endif
}

void AudioSource::SetRolloffFactor(float factor) {
    m_rolloffFactor = factor;
#ifdef HAVE_OPENAL
    if (m_sourceID) {
        alSourcef(m_sourceID, AL_ROLLOFF_FACTOR, factor);
    }
#endif
}

AudioSource::State AudioSource::GetState() const {
    return m_state;
}

glm::vec3 AudioSource::GetPosition() const {
    return m_position;
}

float AudioSource::GetVolume() const {
    return m_volume;
}

float AudioSource::GetPitch() const {
    return m_pitch;
}

bool AudioSource::IsLooping() const {
    return m_looping;
}

void AudioSource::Cleanup() {
#ifdef HAVE_OPENAL
    if (m_sourceID) {
        alDeleteSources(1, &m_sourceID);
        m_sourceID = 0;
    }
#endif
    m_currentSound.reset();
    m_state = State::Stopped;
}

void AudioSource::UpdateSourceParams() {
#ifdef HAVE_OPENAL
    if (m_sourceID) {
        alSource3f(m_sourceID, AL_POSITION, m_position.x, m_position.y, m_position.z);
        alSource3f(m_sourceID, AL_VELOCITY, m_velocity.x, m_velocity.y, m_velocity.z);
        alSourcef(m_sourceID, AL_GAIN, m_volume);
        alSourcef(m_sourceID, AL_PITCH, m_pitch);
        alSourcei(m_sourceID, AL_LOOPING, m_looping ? AL_TRUE : AL_FALSE);
        alSourcef(m_sourceID, AL_REFERENCE_DISTANCE, m_referenceDistance);
        alSourcef(m_sourceID, AL_MAX_DISTANCE, m_maxDistance);
        alSourcef(m_sourceID, AL_ROLLOFF_FACTOR, m_rolloffFactor);
    }
#endif
}

bool AudioSource::CheckALError(const char* context) const {
#ifdef HAVE_OPENAL
    ALenum error = alGetError();
    if (error != AL_NO_ERROR) {
        return false;
    }
#endif
    return true;
}
