// 
// NEWorld: Chunk.h
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
#include "Hitbox.h"
#include "Blocks.h"
#include "Frustum.h"
#include "Renderer.h"
#include "WorldSave.h"

class Object;

namespace World {
    extern std::string worldname;
    extern Brightness skylight;
    extern std::unique_ptr<WorldSave> worldSave;

    class Chunk;

    // This function will only be used on client side, so just checking the lower 20 bits will be enough
    constexpr ChunkID getChunkID(int x, int y, int z) noexcept {
        return (ChunkID(x & 0xFFFFF) << 40) + (ChunkID(y & 0xFFFFF) << 20) + (z & 0xFFFFF);
    }

    void explode(int x, int y, int z, int r, Chunk* c);

    class Chunk {
    public:
        //竟然一直都没有构造函数/析构函数 还要手动调用Init...我受不了啦(╯‵□′)╯︵┻━┻ --Null
        //2333 --qiaozhanrong
        Chunk(int cxi, int cyi, int czi, ChunkID idi);
        ~Chunk();
        int32_t cx, cy, cz;
        bool mIsEmpty, mIsUpdated, mIsRenderBuilt, mIsModified, mIsDetailGenerated;
        ChunkID id;
        Renderer::VertexBuffer vbo[4];
        double loadAnim;
        bool visible{};

        void load(bool initIfEmpty = true);
        void buildRender();

        Vec3i getPos() const noexcept { return Vec3i{ cx, cy, cz }; }

        Block getBlock(int x, int y, int z) const { return mBlocks[x << 8 ^ y << 4 ^ z]; }

        Brightness getBrightness(int x, int y, int z) const { return mBrightness[x << 8 ^ y << 4 ^ z]; }

        void setblock(int x, int y, int z, Block iblock) {
            if (iblock == Blocks::TNT) {
                explode(cx * 16 + x, cy * 16 + y, cz * 16 + z, 8, this);
                return;
            }
            mBlocks[x << 8 ^ y << 4 ^ z] = iblock;
            mIsModified = true;
        }

        void setbrightness(int x, int y, int z, Brightness ibrightness) {
            mBrightness[x << 8 ^ y << 4 ^ z] = ibrightness;
            mIsModified = true;
        }

        static void setRelativeBase(double x, double y, double z, Frustum& frus) {
            relBaseX = x;
            relBaseY = y;
            relBaseZ = z;
            TestFrustum = frus;
        }

        Frustum::ChunkBox getRelativeAABB() const;
        void calcVisible() { visible = TestFrustum.frustumTest(getRelativeAABB()); }
    private:
        Block mBlocks[4096]{};
        Brightness mBrightness[4096]{};
        Hitbox::AABB mAABB{};
        static double relBaseX, relBaseY, relBaseZ;
        static Frustum TestFrustum;
        bool LoadFromFile(); //返回true代表区块文件打开成功
        void SaveToFile();
        Hitbox::AABB getBaseAABB() const;
        void buildTerrain(bool initIfEmpty = true);
        void buildDetail();
        void build(bool initIfEmpty = true);
    };
}
