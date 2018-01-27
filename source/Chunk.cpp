// 
// NEWorld: Chunk.cpp
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

#include "Chunk.h"
#include "WorldGen.h"
#include "World.h"
#include "Blocks.h"
#include <algorithm>
#include <fstream>

namespace ChunkRenderer {
    void renderChunk(World::Chunk* c);
    void mergeFaceRender(World::Chunk* c);
    void renderDepthModel(World::Chunk* c);
}

namespace World {
    struct HMapManager {
        int h[16][16]{0};
        int low{}, high{};

        HMapManager() = default;;

        HMapManager(int cx, int cz) {
            int l = 0xEFFFFFFF, hi = WorldGen::WaterLevel;
            for (int x = 0; x < 16; ++x) {
                for (int z = 0; z < 16; ++z) {
                    auto he = hMap.getHeight(cx * 16 + x, cz * 16 + z);
                    if (he < l) l = he;
                    if (he > hi) hi = he;
                    h[x][z] = he;
                }
            }
            low = (l - 21) / 16, high = (hi + 16) / 16;
        }
    };

    double Chunk::relBaseX, Chunk::relBaseY, Chunk::relBaseZ;
    Frustum Chunk::TestFrustum;

    Chunk::Chunk(int cxi, int cyi, int czi, ChunkID idi) : cx(cxi), cy(cyi), cz(czi), mIsEmpty(false),
                                                           mIsUpdated(false), mIsRenderBuilt(false), mIsModified(false),
                                                           mIsDetailGenerated(false), id(idi),
                                                           loadAnim(0.0) { mAABB = getBaseAABB(); }

    Chunk::~Chunk() {
        unloadedChunksCount++;
        SaveToFile();
        mIsUpdated = false;
        unloadedChunks++;
    }

    void Chunk::buildTerrain(bool initIfEmpty) {
        //Fast generate parts
        //Part1 out of the terrain bound
        if (cy > 16) {
            mIsEmpty = true;
            if (initIfEmpty) {
                std::fill_n(mBlocks, 4096, Blocks::AIR);
                std::fill_n(mBrightness, 4096, skylight);
            }
            return;
        }
        if (cy < 0) {
            mIsEmpty = true;
            if (initIfEmpty) {
                std::fill_n(mBlocks, 4096, Blocks::AIR);
                std::fill_n(mBrightness, 4096, BrightnessMin);
            }
            return;
        }

        //Part2 out of geomentry area
        HMapManager cur(cx, cz);
        if (cy > cur.high) {
            mIsEmpty = true;
            if (initIfEmpty) {
                std::fill_n(mBlocks, 4096, Blocks::AIR);
                std::fill_n(mBrightness, 4096, skylight);
            }
        }
        if (cy < cur.low) {
            std::fill_n(mBlocks, 4096, Blocks::ROCK);
            std::fill_n(mBrightness, 4096, BrightnessMin);
            if (cy == 0)
                for (int x = 0; x < 16; x++)
                    for (int z = 0; z < 16; z++)
                        mBlocks[x * 256 + z] = Blocks::BEDROCK;
            mIsEmpty = false;
            return;
        }

        //Normal Calc
        //Init
        std::fill_n(mBlocks, 4096, Blocks::AIR);
        std::fill_n(mBrightness, 4096, 0);

        int x, z, h = 0, sh = 0, wh = 0;
        int minh, maxh, cur_br;

        mIsEmpty = true;
        sh = WorldGen::WaterLevel + 2 - (cy << 4);
        wh = WorldGen::WaterLevel - (cy << 4);

        for (x = 0; x < 16; ++x) {
            for (z = 0; z < 16; ++z) {
                int base = (x << 8) + z;
                h = cur.h[x][z] - (cy << 4);
                if (h >= 0 || wh >= 0) mIsEmpty = false;
                if (h > sh && h > wh + 1) {
                    //Grass layer
                    if (h >= 0 && h < 16) mBlocks[(h << 4) + base] = Blocks::GRASS;
                    //Dirt layer
                    maxh = std::min(std::max(0, h), 16);
                    for (int y = std::min(std::max(0, h - 5), 16); y < maxh; ++y)
                        mBlocks[(y << 4) + base] = Blocks::
                            DIRT;
                }
                else {
                    //Sand layer
                    maxh = std::min(std::max(0, h + 1), 16);
                    for (int y = std::min(std::max(0, h - 5), 16); y < maxh; ++y)
                        mBlocks[(y << 4) + base] = Blocks::
                            SAND;
                    //Water layer
                    minh = std::min(std::max(0, h + 1), 16);
                    maxh = std::min(std::max(0, wh + 1), 16);
                    cur_br = BrightnessMax - (WorldGen::WaterLevel - (maxh - 1 + (cy << 4))) * 2;
                    if (cur_br < BrightnessMin) cur_br = BrightnessMin;
                    for (int y = maxh - 1; y >= minh; --y) {
                        mBlocks[(y << 4) + base] = Blocks::WATER;
                        mBrightness[(y << 4) + base] = (Brightness)cur_br;
                        cur_br -= 2;
                        if (cur_br < BrightnessMin) cur_br = BrightnessMin;
                    }
                }
                //Rock layer
                maxh = std::min(std::max(0, h - 5), 16);
                for (int y = 0; y < maxh; ++y) mBlocks[(y << 4) + base] = Blocks::ROCK;
                //Air layer
                for (int y = std::min(std::max(0, std::max(h + 1, wh + 1)), 16); y < 16; ++y) {
                    mBlocks[(y << 4) + base] = Blocks::AIR;
                    mBrightness[(y << 4) + base] = skylight;
                }
                //Bedrock layer (overwrite)
                if (cy == 0) mBlocks[base] = Blocks::BEDROCK;
            }
        }
    }

    void Chunk::buildDetail() {
        /*
        int index = 0;
        for (int x = 0; x < 16; x++) {
            for (int y = 0; y < 16; y++) {
                for (int z = 0; z < 16; z++) {
                    //Tree
                    if (mBlocks[index] == Blocks::GRASS && rnd() < 0.005) {
                        buildtree(cx * 16 + x, cy * 16 + y, cz * 16 + z);
                    }
                    index++;
                }
            }
        }
        */
    }

    void Chunk::build(bool initIfEmpty) {
        buildTerrain(initIfEmpty);
        if (!mIsEmpty)
            buildDetail();
        mIsModified = false;
    }

    void Chunk::load(bool initIfEmpty) {
        if (!LoadFromFile())
            build(initIfEmpty);
        if (!mIsEmpty)
            mIsUpdated = true;
    }

    bool Chunk::LoadFromFile() {
        return worldSave->load({cx, cy, cz}, [this](std::fstream& stream) {
            read(mBlocks, stream, 4096 * sizeof(Block));
            read(mBrightness, stream, 4096 * sizeof(Brightness));
            read(&mIsDetailGenerated, stream, sizeof(bool));
        });
    }

    void Chunk::SaveToFile() {
        if (mIsModified) {
            worldSave->save({cx, cy, cz}, [this](std::fstream& stream) {
                write(mBlocks, stream, 4096 * sizeof(Block));
                write(mBrightness, stream, 4096 * sizeof(Brightness));
                write(&mIsDetailGenerated, stream, sizeof(bool));
            });
        }
    }

    void Chunk::buildRender() {
        for (int8_t x = -1; x <= 1; x++)
            for (int8_t y = -1; y <= 1; y++)
                for (int8_t z = -1; z <= 1; z++)
                    if (x && y && z)
                        if (!chunkLoaded(cx + x, cy + y, cz + z))
                            return;

        rebuiltChunks++;
        updatedChunks++;

        if (!mIsRenderBuilt) {
            mIsRenderBuilt = true;
            loadAnim = cy * 16.0f + 16.0f;
        }

        if (MergeFace)
            ChunkRenderer::mergeFaceRender(this);
        else
            ChunkRenderer::renderChunk(this);
        if (Renderer::AdvancedRender)
            ChunkRenderer::renderDepthModel(this);

        mIsUpdated = false;
    }

    Hitbox::AABB Chunk::getBaseAABB() const {
        Hitbox::AABB ret{};
        ret.xmin = cx * 16 - 0.5;
        ret.ymin = cy * 16 - 0.5;
        ret.zmin = cz * 16 - 0.5;
        ret.xmax = cx * 16 + 16 - 0.5;
        ret.ymax = cy * 16 + 16 - 0.5;
        ret.zmax = cz * 16 + 16 - 0.5;
        return ret;
    }

    Frustum::ChunkBox Chunk::getRelativeAABB() const {
        Frustum::ChunkBox ret{};
        ret.xmin = static_cast<float>(mAABB.xmin - relBaseX);
        ret.xmax = static_cast<float>(mAABB.xmax - relBaseX);
        ret.ymin = static_cast<float>(mAABB.ymin - loadAnim - relBaseY);
        ret.ymax = static_cast<float>(mAABB.ymax - loadAnim - relBaseY);
        ret.zmin = static_cast<float>(mAABB.zmin - relBaseZ);
        ret.zmax = static_cast<float>(mAABB.zmax - relBaseZ);
        return ret;
    }
}
