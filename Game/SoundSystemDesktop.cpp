// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#include "SoundSystem.h"
#include <cstdio>
#include <AL/al.h>
#include <AL/alc.h>
#include <vorbis/vorbisfile.h>
#include <cstdint>
#include <string>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <list>

namespace Chewman
{

namespace
{

class OpenALProcessor
{
public:
    ~OpenALProcessor()
    {
        if (_device)
        {
            alcMakeContextCurrent(nullptr);
            alcDestroyContext(_context);
            alcCloseDevice(_device);
            _device = nullptr;
        }
    }

    static OpenALProcessor* getInstance()
    {
        if (_instance == nullptr)
        {
            _instance = new OpenALProcessor();
        }
        return _instance;
    }

    bool isActive() const
    {
        return _device != nullptr;
    }

    uint32_t registerSound(const std::string& sound, bool looped = false)
    {
        // Ogg loading code is based on http://github.com/tilkinsc
        ALuint buffer = 0;
        FILE* fp = nullptr;
        OggVorbis_File vf {};
        vorbis_info * vi = nullptr;
        ALenum format = 0;
        std::vector<int16_t> bufferData;

        // open the file in read binary mode
        // TODO: use filesystem file read
        fp = fopen(sound.c_str(), "rb");
        if(fp == nullptr)
        {
            std::cerr << "Can't open sound file " << sound << std::endl;
            return -1;
        }

        alGenBuffers(1, &buffer);

        if(ov_open_callbacks(fp, &vf, nullptr, 0, OV_CALLBACKS_NOCLOSE) < 0)
        {
            std::cerr << "Ogg sound file is not valid: " << sound << std::endl;
            fclose(fp);
            return -1;
        }

        vi = ov_info(&vf, -1);
        format = vi->channels == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;

        size_t data_len = ov_pcm_total(&vf, -1) * vi->channels * 2;
        bufferData.resize(data_len);

        for (long size = 0, offset = 0, sel = 0;
             (size = ov_read(&vf, (char*) bufferData.data() + offset, 4096, 0, sizeof(int16_t), 1, (int*) &sel)) != 0;
             offset += size) {
            if(size < 0)
            {
                std::cerr << "Incorrect ogg file: " << sound << std::endl;
                fclose(fp);
                ov_clear(&vf);
                return -1;
            }
        }

        alBufferData(buffer, format, bufferData.data(), data_len, vi->rate);
        fclose(fp);
        ov_clear(&vf);

        std::array<ALuint, 4> sources {};
        alGenSources((ALuint) 4, sources.data());
        for (auto i = 0; i < 4; ++i)
        {
            alSourcef(sources[i], AL_PITCH, 1);
            alSourcef(sources[i], AL_GAIN, 1);
            alSource3f(sources[i], AL_POSITION, 0, 0, 0);
            alSource3f(sources[i], AL_VELOCITY, 0, 0, 0);
            alSourcei(sources[i], AL_LOOPING, looped ? AL_TRUE : AL_FALSE);
            alSourcei(sources[i], AL_BUFFER, buffer);
        }

        std::list<ALuint> sourcesList(sources.begin(), sources.end());
        _sourcesMap[_lastSound++] = std::move(sourcesList);

        return _lastSound - 1;
    }

    void playSound(uint32_t id)
    {
        auto& sourcesList = _sourcesMap[id];
        auto currentSource = sourcesList.front();
        ALenum state;
        alGetSourcei(currentSource, AL_SOURCE_STATE, &state);
        if (state != AL_PLAYING)
        {
            alSourcePlay(currentSource);
            sourcesList.pop_front();
            sourcesList.push_back(currentSource);
        }
    }

    void stopSound(uint32_t id)
    {
        auto& sourcesList = _sourcesMap[id];
        for (auto source : sourcesList)
            alSourceStop(source);
    }

    void setVolume(uint32_t id, float value)
    {
        auto& sourcesList = _sourcesMap[id];
        for (auto source : sourcesList)
            alSourcef(source, AL_GAIN, value);
    }

private:
    static OpenALProcessor* _instance;

    OpenALProcessor()
    {
        _device = alcOpenDevice(nullptr);
        if (!_device) {
            return;
        }

        _context = alcCreateContext(_device, nullptr);
        if (!alcMakeContextCurrent(_context))
        {
            alcCloseDevice(_device);
            _device = nullptr;
        }
    }

    ALCdevice* _device = nullptr;
    ALCcontext* _context = nullptr;

    std::unordered_map<uint32_t, std::list<ALuint>> _sourcesMap;
    int _lastSound = 0;
};

OpenALProcessor* OpenALProcessor::_instance = nullptr;

class OpenALSound : public Sound
{
public:
    explicit OpenALSound(uint32_t id)
    {
        _id = id;
    }

    void play() override
    {
        OpenALProcessor::getInstance()->playSound(_id);
    }

    void stop() override
    {
        OpenALProcessor::getInstance()->stopSound(_id);
    }

    void setVolume(float volume) override
    {
        OpenALProcessor::getInstance()->setVolume(_id, volume);
    }

private:
    uint32_t _id;
};


} // anon namespace

SoundSystem* SoundSystem::_instance = nullptr;

SoundSystem::SoundSystem() = default;
SoundSystem::~SoundSystem() = default;

SoundSystem* SoundSystem::getInstance()
{
    if (_instance == nullptr)
    {
        _instance = new SoundSystem();
    }
    return _instance;
}

bool SoundSystem::isLoaded() const
{
    return OpenALProcessor::getInstance()->isActive();
}

std::shared_ptr<Sound> SoundSystem::createSound(const std::string& filename, bool loop)
{
    return std::make_shared<OpenALSound>(OpenALProcessor::getInstance()->registerSound(filename, loop));
}

void SoundSystem::initBackgroundMusic(const std::string& filename)
{
    _bgm = createSound(filename, true);
}

void SoundSystem::startBackgroundMusic()
{
    if (_bgm != nullptr && !_isBgmPlaying)
    {
        _bgm->play();
        _isBgmPlaying = true;
    }
}

void SoundSystem::stopBackgroundMusic()
{
    if (_bgm != nullptr)
    {
        _bgm->stop();
        _isBgmPlaying = false;
    }
}

void SoundSystem::setBackgroundMusicVolume(float volume)
{
    if (_bgm != nullptr)
        _bgm->setVolume(volume);
}

} // namespace Chewman