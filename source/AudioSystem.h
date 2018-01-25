// 
// NWShared: AudioSystem.h
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

#pragma once
#include "StdInclude.h"
#include "Math/Vector.h"
#include "Utils/Concept.h"
#include "Config/Config.hpp"

struct NW_API AudioSystemSettings final {
    float BGMGain;
    float effectGain;
    float GUIGain;
    float rolloffFactor;
    float maxDistance;
    float dopplerFactor;
    AudioSystemSettings();
};

class AudioSystemImpl;

constexpr auto SOSAir = 343.3f, SOSWater = 1473.0f;

class NW_API AudioSystem final : Singletion {
public:
    void play(const filesystem::path& path, Vec3f pos, Vec3f velocity);
    void play(const filesystem::path& path);
    void update(Vec3f pos = {}, Vec3f velocity = {}, Vec3f lookAt = {0.0f, 0.0f, -1.0f}, Vec3f up = {0.0f, 1.0f, 0.0f});
    void setSettings(AudioSystemSettings settings);
    AudioSystemSettings getSettings() const;
    void setSpeedOfSound(float speed);
    static AudioSystem& getInstance() noexcept;
private:
    std::unique_ptr<AudioSystemImpl> mPimpl;
    AudioSystem();
};

NW_API AudioSystem& getAudioSystem();
