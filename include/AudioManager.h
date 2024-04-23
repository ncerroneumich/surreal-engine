// AudioManager.h
#pragma once

#include <SDL2/SDL.h>
#include <string>
#include <unordered_map>


struct AudioData {
    SDL_AudioSpec spec;
    Uint8* buffer = nullptr;
    Uint32 length = 0;
    Uint32 position = 0;
    bool isPlaying = false;
    bool loops = false;
    float volume = 1.0f;
};

struct CachedAudio {
    SDL_AudioSpec spec;
    Uint8* buffer;
    Uint32 length;
};

class AudioManager {
public:
    static void Initialize();
    static bool LoadWAV(const std::string& filename, int channel);
    static void Play(const std::string& filename, int channel, bool loops);
    static void Stop(int channel);
    static bool IsPlaying(int channel);
    static void AudioCallback(void* userdata, Uint8* stream, int len);
    static float GetPosition(int channel);
    static void SetVolume(int channel, float volume);

private:
    static inline std::string audioFolderPath = "resources/audio/";
    static constexpr size_t numChannels = 64;
    static inline SDL_AudioDeviceID audioDevice;
    static inline SDL_AudioSpec audioSpec;
    static inline AudioData audioData[numChannels];
    static inline std::unordered_map<std::string, CachedAudio> audioCache;
};
