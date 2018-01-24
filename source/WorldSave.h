// 
// NEWorld: WorldSave.h
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
#include "Typedefs.h"
#include "Math/Vector.h"
#include <unordered_map>
#include <fstream>
#include <functional>

namespace World {
    constexpr auto chunkSize = 4096 * (sizeof(Block) + sizeof(Brightness)) + sizeof(bool);

    void read(void* ptr, std::fstream& stream, uint64_t size);
    void write(const void* ptr, std::fstream& stream, uint64_t size);

    class WorldSave final {
    private:
        struct PosHasher final {
            using argument_type = Vec3i;
            using result_type = ChunkID;
            result_type operator()(argument_type key) const;
        };

        std::unordered_map<Vec3i, uint64_t, PosHasher> mIndex;
        std::fstream mIndexStream, mStream;
    public:
        WorldSave(const filesystem::path& path);
        bool load(Vec3i pos, const std::function<void(std::fstream&)>& loadFunc);
        void save(Vec3i pos, const std::function<void(std::fstream&)>& saveFunc);
    };
}
