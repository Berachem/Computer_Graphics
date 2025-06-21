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

class Sound;

// Source audio 3D
class AudioSource {
public:
    enum class State {
        Stopped,
        Playing,
        Paused
    };

    AudioSource();
    ~AudioSource();
    void Play(std::shared_ptr<Sound> sound, bool loop = false);
    void Pause();
    void Resume();
    void Stop();
    void SetPosition(const glm::vec3& position);
    void SetVelocity(const glm::vec3& velocity);
    void SetVolume(float volume);
    void SetPitch(float pitch);
    void SetLooping(bool loop);
    void SetReferenceDistance(float distance);
    void SetMaxDistance(float distance);
    void SetRolloffFactor(float factor);
    State GetState() const;
    glm::vec3 GetPosition() const;
    float GetVolume() const;
    float GetPitch() const;
    bool IsLooping() const;
    void Cleanup();
private:
    AudioSourceID m_sourceID;
    glm::vec3 m_position;
    glm::vec3 m_velocity;
    float m_volume;
    float m_pitch;
    bool m_looping;
    float m_referenceDistance;
    float m_maxDistance;
    float m_rolloffFactor;
    State m_state = State::Stopped;
    std::shared_ptr<Sound> m_currentSound;
    void UpdateSourceParams();
    bool CheckALError(const char* context) const;
};

#endif
