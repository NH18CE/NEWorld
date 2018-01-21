// 
// NEWorld: AudioSystem.cpp
// NEWorld: A Free Game with Similar Rules to Minecraft.
// Copyright (C) 2015-2018 NEWorld Team
// 
// NEWorld is free software: you can redistribute it and/or modify it 
// under the terms of the GNU Lesser General Public License as published
// by the Free Software Foundation, either version 3 of the License, or 
// (at your option) any later version.
// 
// NEWorld is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY 
// or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General 
// Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with NEWorld.  If not, see <http://www.gnu.org/licenses/>.
// 

#include "AudioSystem.h"
#include <AL/al.h>
#include <AL/alc.h>
#pragma warning(push,0)
#include "stb_vorbis.c"
#pragma warning(pop)
#include <unordered_map>

constexpr auto defaultFactor = 0.0f, defaultDistance=100.0f;
constexpr auto moved = 0xffffffff;

AudioSystem & getAudioSystem() {
    static AudioSystem system;
    return system;
}

class Buffer final :Uncopyable {
private:
    ALuint mBuffer;
public:
    Buffer(const filesystem::path& path);
    Buffer(Buffer&& rhs);
    ALuint get() const;
    ~Buffer();
};

class Source final :Uncopyable {
private:
    ALuint mSource;
public:
    Source();
    Source(Source&& rhs);
    void play(const Buffer& buffer);
    void setGain(float gain);
    void setFactor(float rolloffFactor,float maxDistance);
    void set3D(Vec3f pos, Vec3f velocity);
    bool isPlaying() const;
    ~Source();
};

class AudioSystemImpl final:Singletion {
private:
    AudioSystemSettings mSettings;
    std::unordered_map<std::wstring, Buffer> mBufferCache;
    std::vector<Source> mSources;
    std::unique_ptr<Source> mBGM;
    std::vector<Buffer> mBGMBuffers;
    ALCdevice* mDevice;
    ALCcontext* mContext;
    Buffer& getBuffer(const filesystem::path& path);
    Source& getSource();
public:
    AudioSystemImpl();
    ~AudioSystemImpl();
    void update();
    void play(const filesystem::path & path, Vec3f pos, Vec3f velocity);
    void play(const filesystem::path & path);
    void setSettings(AudioSystemSettings settings);
    AudioSystemSettings getSettings() const;
};

AudioSystem::AudioSystem():mPimpl(std::make_unique<AudioSystemImpl>()){}

void AudioSystem::play(const filesystem::path & path, Vec3f pos, Vec3f velocity) {
    mPimpl->play(path, pos, velocity);
}

void AudioSystem::play(const filesystem::path & path) {
    mPimpl->play(path);
}

void AudioSystem::update(Vec3f pos, Vec3f velocity,Vec3f lookAt,Vec3f up) {
    alListenerfv(AL_POSITION,pos.data);
    alListenerfv(AL_VELOCITY, velocity.data);
    Vec3f ori[2] = { lookAt,up };
    alListenerfv(AL_ORIENTATION, ori->data);
    mPimpl->update();
}

void AudioSystem::setSettings(AudioSystemSettings settings) {
    mPimpl->setSettings(settings);
}

AudioSystemSettings AudioSystem::getSettings() const {
    return mPimpl->getSettings();
}

void AudioSystem::setSpeedOfSound(float speed) {
    alSpeedOfSound(speed);
}

AudioSystemSettings::AudioSystemSettings()
    :BGMGain(1.0f),effectGain(1.0f),GUIGain(1.0f),
    rolloffFactor(defaultFactor),maxDistance(defaultDistance) {}

Buffer & AudioSystemImpl::getBuffer(const filesystem::path & path) {
    auto key = path.wstring();
    auto it = mBufferCache.find(key);
    if (it == mBufferCache.cend())
        it = mBufferCache.emplace(key, Buffer(path)).first;
    return it->second;
}

Source & AudioSystemImpl::getSource() {
    for (auto&& src : mSources)
        if (!src.isPlaying())
            return src;
    return mSources.emplace_back();
}

AudioSystemImpl::AudioSystemImpl() {
    mDevice=alcOpenDevice(nullptr);
    if (!mDevice)
        throw std::exception("Failed to open default device.");
    mContext = alcCreateContext(mDevice,nullptr);
    if (!mContext) {
        alcCloseDevice(mDevice);
        throw std::exception("Failed to create a context.");
    }
    alcMakeContextCurrent(mContext);
    alDistanceModel(AL_INVERSE_DISTANCE_CLAMPED);
    alDopplerFactor(1.0f);
    alSpeedOfSound(SOSAir);
    filesystem::path BGMDir("Audio/BGM");
    for (auto&& it : filesystem::directory_iterator(BGMDir)) {
        auto path = it.path();
        if (it.status().type()==filesystem::file_type::regular && path.extension()==".ogg")
            mBGMBuffers.emplace_back(path);
    }
    mBGM = std::make_unique<Source>();
    update();
}

AudioSystemImpl::~AudioSystemImpl() {
    alcMakeContextCurrent(nullptr);
    alcDestroyContext(mContext);
    alcCloseDevice(mDevice);
}

void AudioSystemImpl::update() {
    if (!mBGM->isPlaying() && !mBGMBuffers.empty()) {
        auto id = rand() % mBGMBuffers.size();
        mBGM->play(mBGMBuffers[id]);
    }
}

void AudioSystemImpl::play(const filesystem::path & path, Vec3f pos, Vec3f velocity) {
    auto& src = getSource();
    src.play(getBuffer(path));
    src.setFactor(mSettings.rolloffFactor,mSettings.maxDistance);
    src.setGain(mSettings.effectGain);
    src.set3D(pos,velocity);
}

void AudioSystemImpl::play(const filesystem::path & path) {
    auto& src = getSource();
    src.play(getBuffer(path));
    src.setFactor(defaultFactor,defaultDistance);
    src.setGain(mSettings.GUIGain);
    src.set3D({}, {});
}

void AudioSystemImpl::setSettings(AudioSystemSettings settings) {
    mSettings = settings;
    mBGM->setGain(mSettings.BGMGain);
    mBGM->setFactor(defaultFactor, defaultDistance);
    alDopplerFactor(mSettings.dopplerFactor);
}

AudioSystemSettings AudioSystemImpl::getSettings() const {
    return mSettings;
}

Buffer::Buffer(const filesystem::path & path) {
    alGenBuffers(1,&mBuffer);
    if (alGetError() != AL_NO_ERROR)
        throw std::exception("Failed to create a buffer.");
    int channels, sampleRate;
    short* output = nullptr;
    auto res=stb_vorbis_decode_filename(path.string().c_str(),&channels,&sampleRate,&output);
    if(res==-1)throw std::exception("Failed to load this file.");
    ALenum format = 0;
    if (channels == 1)format = AL_FORMAT_MONO16;
    else if (channels == 2)format = AL_FORMAT_STEREO16;
    if (format)alBufferData(mBuffer, format, output, res*channels * sizeof(short), sampleRate);
    else {
        free(output);
        throw std::exception("Unknown format.");
    }
    free(output);
    if(alGetError()!=AL_NO_ERROR)
        throw std::exception("Failed to set PCM data.");
}

Buffer::Buffer(Buffer && rhs):mBuffer(rhs.mBuffer) {
    rhs.mBuffer = moved;
}

ALuint Buffer::get() const {
    return mBuffer;
}

Buffer::~Buffer() {
    if(mBuffer!=moved)alDeleteBuffers(1, &mBuffer);
}

Source::Source() {
    alGenSources(1,&mSource);
    if (alGetError() != AL_NO_ERROR)
        throw std::exception("Failed to create a source.");
}

Source::Source(Source && rhs):mSource(rhs.mSource) {
    rhs.mSource = moved;
}

void Source::play(const Buffer & buffer) {
    alSourceStop(mSource);
    alSourcei(mSource,AL_BUFFER,buffer.get());
    alSourcei(mSource, AL_LOOPING, false);
    alSourcePlay(mSource);
}

void Source::setGain(float gain) {
    alSourcef(mSource, AL_GAIN, gain);
}

void Source::setFactor(float rolloffFactor,float maxDistance) {
    alSourcef(mSource,AL_MAX_DISTANCE,maxDistance);
    alSourcef(mSource, AL_REFERENCE_DISTANCE, 1.0f);
    alSourcef(mSource, AL_ROLLOFF_FACTOR, rolloffFactor);
}

void Source::set3D(Vec3f pos, Vec3f velocity) {
    alSourcefv(mSource, AL_POSITION, pos.data);
    alSourcefv(mSource, AL_VELOCITY, velocity.data);
}

bool Source::isPlaying() const {
    ALint state;
    alGetSourcei(mSource,AL_SOURCE_STATE,&state);
    return state == AL_PLAYING;
}

Source::~Source() {
    if(mSource!=moved)alDeleteSources(1,&mSource);
}
