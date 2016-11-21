/*
* NEWorld: A free game with similar rules to Minecraft.
* Copyright (C) 2016 NEWorld Team
*
* This file is part of NEWorld.
* NEWorld is free software: you can redistribute it and/or modify
* it under the terms of the GNU Lesser General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* NEWorld is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with NEWorld.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef CHUNK_H_
#define CHUNK_H_

#include <atomic>
#include <chrono>
#include "common/vec3.h"
#include "common/debug.h"
#include "common/nwexport.h"
#include "nwblock.h"

using ChunkGenerator = void NWAPICALL(const Vec3i*, BlockData*, int);


class NWCOREAPI Chunk
{
public:
	// Chunk size
    static bool ChunkGeneratorLoaded;
    static ChunkGenerator *ChunkGen;
	static constexpr int SizeLog2() { return 5; }
	static constexpr int Size(){ return 0b100000; };

    explicit Chunk(const Vec3i& position) : mPosition(position) {}
    virtual ~Chunk() {}

    // Get chunk position
    const Vec3i& getPosition() const
    {
        return mPosition;
    }

    // Get chunk updated flag
    bool isUpdated() const
    {
        return mUpdated;
    }

    // Set chunk updated flag
    void setUpdated(bool updated)
    {
        mUpdated = updated;
    }

    // Get block data in this chunk
    BlockData getBlock(const Vec3i& pos) const
    {
        Assert(pos.x >= 0 && pos.x < Size() && pos.y >= 0 && pos.y < Size() && pos.z >= 0 && pos.z < Size());
        return mBlocks[pos.x * Size() * Size() + pos.y * Size() + pos.z];
    }

    // Get block reference in this chunk
    BlockData& getBlock(const Vec3i& pos)
    {
        Assert(pos.x >= 0 && pos.x < Size() && pos.y >= 0 && pos.y < Size() && pos.z >= 0 && pos.z < Size());
        return mBlocks[pos.x * Size() * Size() + pos.y * Size() + pos.z];
    }

    // Get block pointer
    BlockData* getBlocks()
    {
        return mBlocks;
    }
    const BlockData* getBlocks() const
    {
        return mBlocks;
    }

    // Set block data in this chunk
    void setBlock(const Vec3i& pos, BlockData block)
    {
        Assert(pos.x >= 0 && pos.x < Size() && pos.y >= 0 && pos.y < Size() && pos.z >= 0 && pos.z < Size());
        mBlocks[pos.x * Size() * Size() + pos.y * Size() + pos.z] = block;
        mUpdated = true;
    }

    // Build chunk
    void build(int daylightBrightness);

    // Reference Counting
    void increaseWeakRef();
    void decreaseWeakRef();
    void increaseStrongRef();
    void decreaseStrongRef();
    bool checkReleaseable() const;

private:
    Vec3i mPosition;
    BlockData mBlocks[0b1'00000'00000'00000];
    bool mUpdated = false;
	// For Garbage Collection
	long long mReferenceCount;
	std::chrono::steady_clock::time_point mLastRequestTime;
    std::atomic<int> mRefrenceCount{0}, mWeakRefrenceCount{0};
};

#endif // !CHUNK_H_
