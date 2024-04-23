// AudioManager.cpp
#include "AudioManager.h"
#include <cstring>
#include <iostream>


void AudioManager::Initialize() {
    // Initialize the SDL audio systems
    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        SDL_Log("Failed to initialize SDL Audio: %s", SDL_GetError());
        return;
    }
    
    // Define our desired spec for audio (all audio played should meet this spec)
    SDL_AudioSpec desiredSpec;
    SDL_zero(desiredSpec);
    desiredSpec.freq = 44100;  // Sampling rate
    desiredSpec.format = AUDIO_S16SYS;
    desiredSpec.channels = 2;  // Mono
    desiredSpec.samples = 1024;  // Buffer size
    desiredSpec.callback = AudioCallback;  // Pointer to our callback function

    // Open the audio device
    audioDevice = SDL_OpenAudioDevice(NULL, 0, &desiredSpec, &audioSpec, 0);
    if (audioDevice == 0) {
        SDL_Log("Failed to open audio device: %s", SDL_GetError());
    }
    else {
        SDL_PauseAudioDevice(audioDevice, 1);  // Pause audio device
    }
}

bool AudioManager::LoadWAV(const std::string& filename, int channel) {
    std::string filePath = audioFolderPath + filename + ".wav";
    auto cachedAudio = audioCache.find(filename);

    if (cachedAudio != audioCache.end()) {
        audioData[channel] = { cachedAudio->second.spec, cachedAudio->second.buffer, cachedAudio->second.length, 0 };
        return true;
    }

    SDL_AudioSpec wavSpec;
    Uint32 wavLength;
    Uint8* wavBuffer;

    if (SDL_LoadWAV(filePath.c_str(), &wavSpec, &wavBuffer, &wavLength) == NULL) {
        return false;
    }

    audioCache[filename] = { wavSpec, wavBuffer, wavLength };
    audioData[channel] = { wavSpec, wavBuffer, wavLength, 0 };

    return true;
}


void AudioManager::AudioCallback(void* userdata, Uint8* stream, int len) 
{
    std::memset(stream, 0, len);

    for (size_t i = 0; i < numChannels; i++) 
    {
        if (audioData[i].buffer && audioData[i].isPlaying) 
        {
            Uint32 bytesMixed = 0;
            while (bytesMixed < static_cast<Uint32>(len))
            {
                Uint32 bytesRemaining = audioData[i].length - audioData[i].position;
                Uint32 bytesToMix = std::min(bytesRemaining, static_cast<Uint32>(len) - bytesMixed);

                SDL_MixAudioFormat(stream + bytesMixed, audioData[i].buffer + audioData[i].position, audioData[i].spec.format, bytesToMix, static_cast<int>(audioData[i].volume * SDL_MIX_MAXVOLUME));

                audioData[i].position += bytesToMix;
                bytesMixed += bytesToMix;


                if (audioData[i].position >= audioData[i].length)
                {
                    if (audioData[i].loops) {
                        audioData[i].position = 0;
                    }
                    else {
                        audioData[i].isPlaying = false;
                        break;
                    }
                }
            }
        }
    }
}


void AudioManager::Play(const std::string& filename, int channel, bool loops) {
    // Attempt load the wav file

    bool loadResult = LoadWAV(filename, channel);
    if (!loadResult) {
        SDL_Log("Failed to load WAV: %s", SDL_GetError());
        return;
    }

    audioData[channel].isPlaying = true;
    audioData[channel].loops = loops;

    SDL_PauseAudioDevice(audioDevice, 0);
}

void AudioManager::Stop(int channel) {
    audioData[channel].isPlaying = false;
    audioData[channel].position = 0;
}

bool AudioManager::IsPlaying(int channel) {
    return audioData[channel].isPlaying;
}

float AudioManager::GetPosition(int channel) {
    return static_cast<float>(audioData[channel].position) / (audioSpec.freq * audioSpec.channels * (SDL_AUDIO_BITSIZE(audioSpec.format) / 8));
}

void AudioManager::SetVolume(int channel, float volume) {
    // Clamp the volume between 0.0 and 1.0
    if (volume < 0.0f) volume = 0.0f;
    if (volume > 1.0f) volume = 1.0f;

    audioData[channel].volume = volume;
}