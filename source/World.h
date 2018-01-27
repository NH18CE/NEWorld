// 
// NEWorld: World.h
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
#include "Definitions.h"
#include "ChunkPtrArray.h"
#include "HeightMap.h"
#include "Chunk.h"
#include "Hitbox.h"
#include "Blocks.h"
#include "Player.h"
#include "Particles.h"
#include "Items.h"
#include "Math/Vector.h"
#include <array>

extern int viewdistance;
class Frsutum;

namespace World {
    // POD ONLY!
    template <class Tk, class Td, size_t Size, template<class> class Compare = std::less>
    class OrderedList {
    public:
        OrderedList() noexcept : mSize(0), mComp() { }

        using ArrayType = std::array<std::pair<Tk, Td>, Size>;
        using Iterator = typename ArrayType::iterator;
        using ConstIterator = typename ArrayType::const_iterator;
        Iterator begin() noexcept { return mList.begin(); }
        ConstIterator begin() const noexcept { return mList.begin(); }
        Iterator end() noexcept { return mList.begin() + mSize; }
        ConstIterator end() const noexcept { return mList.begin() + mSize; }

        void insert(Tk key, Td data) noexcept {
            int first = 0, last = mSize - 1;
            while (first <= last) {
                int middle = (first + last) / 2;
                if (mComp(key, mList[middle].first))
                    last = middle - 1;
                else
                    first = middle + 1;
            }
            if (first <= mSize && first < Size) {
                mSize = std::min(Size, mSize + 1);
                for (int j = Size - 1; j > first; --j)
                    mList[j] = mList[j - 1];
                mList[first] = std::pair<Tk, Td>(key, data);
            }
        }

        void clear() noexcept { mSize = 0; }
    private:
        size_t mSize;
        ArrayType mList;
        Compare<Tk> mComp;
    };

    inline Chunk* emptyChunkPtr = reinterpret_cast<Chunk*>(~0);
    extern std::string worldname;
    constexpr Brightness BrightnessMax = 15; //maximum brightness
    constexpr Brightness BrightnessMin = 2; //Mimimum brightness
    constexpr Brightness BrightnessDec = 1; //Brightness decrease
    inline Brightness skylight = 15; //Sky light level
    constexpr int maxChunkLoads = 64;
    constexpr int maxChunkUnloads = 64;
    constexpr int maxChunkRenders = 1;

    extern Chunk** chunks;
    extern int loadedChunks;
    extern HeightMap hMap;
    extern CPARegionalCache cpArray;

    inline int cloud[128][128];
    inline int rebuiltChunks, rebuiltChunksCount;
    inline int updatedChunks, updatedChunksCount;
    inline int unloadedChunks, unloadedChunksCount;
    inline OrderedList<int, Vec3i, maxChunkLoads> chunkLoadList;
    inline OrderedList<int, Chunk*, maxChunkRenders> chunkBuildRenderList;
    inline OrderedList<int, Chunk*, maxChunkUnloads, std::greater> chunkUnloadList;
    extern std::vector<unsigned int> vbuffersShouldDelete;

    void init();

    Chunk* addChunk(int x, int y, int z);
    void deleteChunk(int x, int y, int z);

    Chunk* getChunkPtr(int x, int y, int z);

    template <class T>
    constexpr auto getChunkPos(T n) noexcept { return n >> 4; }

    template <class T>
    constexpr auto getBlockPos(T n) noexcept { return n & 0xF; }

    inline bool chunkLoaded(int x, int y, int z) { return getChunkPtr(x, y, z); }

    std::vector<Hitbox::AABB> getHitboxes(const Hitbox::AABB& box);
    bool inWater(const Hitbox::AABB& box);

    void updateblock(int x, int y, int z, bool blockchanged, int depth = 0);
    Block getBlock(int x, int y, int z, Block mask = Blocks::AIR, const Chunk* ci = nullptr);
    Brightness getBrightness(int x, int y, int z, Chunk* ci = nullptr);
    void setblock(int x, int y, int z, Block blockname, Chunk* cptr = nullptr);
    void setbrightness(int x, int y, int z, Brightness brightness, Chunk* cptr = nullptr);

    inline void pickleaf() {
        if (rnd() < 0.2) {
            if (rnd() < 0.5)Player::addItem(APPLE);
            else Player::addItem(STICK);
        }
        else { Player::addItem(Blocks::LEAF); }
    }

    void picktree(int x, int y, int z);

    inline void pickblock(int x, int y, int z) {
        if (getBlock(x, y, z) == Blocks::WOOD &&
            ((getBlock(x, y + 1, z) == Blocks::WOOD) || (getBlock(x, y + 1, z) == Blocks::LEAF)) &&
            (getBlock(x, y, z + 1) == Blocks::AIR) && (getBlock(x, y, z - 1) == Blocks::AIR) &&
            (getBlock(x + 1, y, z) == Blocks::AIR) && (getBlock(x - 1, y, z) == Blocks::AIR) &&
            (getBlock(x, y - 1, z) != Blocks::AIR)
        ) { picktree(x, y + 1, z); } //触发砍树模式
        //击打树叶
        if (getBlock(x, y, z) != Blocks::LEAF)Player::addItem(getBlock(x, y, z));
        else pickleaf();

        setblock(x, y, z, Blocks::AIR);
    }


    inline bool chunkInRange(int x, int y, int z, int px, int py, int pz, int dist) {
        //检测给出的chunk坐标是否在渲染范围内
        return !(x < px - dist || x > px + dist - 1 || y < py - dist || y > py + dist - 1 || z < pz - dist || z > pz +
            dist - 1);
    }

    bool chunkUpdated(int x, int y, int z);
    void setChunkUpdated(int x, int y, int z, bool value);
    void sortChunkBuildRenderList(int xpos, int ypos, int zpos);
    void sortChunkLoadUnloadList(int xpos, int ypos, int zpos);
    void calcVisible(double xpos, double ypos, double zpos, Frustum& frus);

    void destroyAllChunks();

    void buildtree(int x, int y, int z);
    void explode(int x, int y, int z, int r, Chunk* c = nullptr);

    void randomChunkUpdation() noexcept;
    void updateChunkLoading() noexcept;
}
