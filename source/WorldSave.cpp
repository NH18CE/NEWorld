// 
// NEWorld: WorldSave.cpp
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

#include "WorldSave.h"
#include "Chunk.h"

namespace World {
    WorldSave::PosHasher::result_type WorldSave::PosHasher::operator()(argument_type key) const {
        return World::getChunkID(key.x, key.y, key.z);
    }

    uint64_t getFileSize(std::fstream& stream) {
        stream.seekg(0, std::ios::end);
        return stream.tellg();
    }

    void read(void* ptr, std::fstream& stream, uint64_t size) { stream.read(reinterpret_cast<char*>(ptr), size); }

    void write(const void* ptr, std::fstream& stream, uint64_t size) {
        stream.write(reinterpret_cast<const char*>(ptr), size);
    }

    constexpr auto flag = std::ios::in | std::ios::out | std::ios::binary | std::ios::app;

    WorldSave::WorldSave(const filesystem::path& path)
        : mIndexStream((path / "chunkIndex").c_str(), flag),
          mStream((path / "chunks").c_str(), flag) {
        if (mIndexStream && mStream) {
            auto size = getFileSize(mIndexStream);
            if (size % sizeof(Vec3i) != 0) throw std::runtime_error("This save is invaild.");
            std::vector<Vec3i> buffer(size / sizeof(Vec3i));
            mIndexStream.seekg(0, std::ios::beg);
            read(buffer.data(), mIndexStream, size);
            uint64_t idx = 0;
            for (auto vec : buffer)
                mIndex[vec] = idx++;
        }
        else throw std::runtime_error("Failed to open this save.");
    }

    bool WorldSave::load(Vec3i pos, const std::function<void(std::fstream&)>& loadFunc) {
        auto it = mIndex.find(pos);
        if (it != mIndex.cend()) {
            mStream.seekg(chunkSize * it->second, std::ios::beg);
            loadFunc(mStream);
            return true;
        }
        return false;
    }

    void WorldSave::save(Vec3i pos, const std::function<void(std::fstream&)>& saveFunc) {
        auto it = mIndex.find(pos);
        if (it == mIndex.end()) {
            mIndexStream.seekp(0, std::ios::end);
            write(&pos, mIndexStream, sizeof(pos));
            auto id = mIndex.size();
            it = mIndex.emplace(pos, id).first;
        }
        mStream.seekp(chunkSize * it->second, std::ios::beg);
        saveFunc(mStream);
    }
}
