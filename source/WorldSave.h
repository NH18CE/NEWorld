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
