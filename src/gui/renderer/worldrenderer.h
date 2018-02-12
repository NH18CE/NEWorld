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

#ifndef WORLDCLIENT_H_
#define WORLDCLIENT_H_

#include <array>
#include <unordered_map>
#include "chunkrenderer.h"
#include "engine/nwstdlib/nworderedlist.hpp"
#include "sync_service/chunkservice.hpp"

const int MaxChunkRenderCount = 4;

/**
 * \brief Manage the VBO of a world. It includes ChunkRenderer.
 */
class WorldRenderer
{
public:
    WorldRenderer(const World& world, int renderDistance)
        : mWorld(world), mRenderDist(renderDistance)
    {
    }

    // Render all chunks
    size_t render(const Vec3i& position) const;

    void registerTask(ChunkService& chunkService, Player& player) noexcept;

private:
    void VAGenerate(const Chunk* chunk);
    void VBOGenerateTask(const Vec3i& position, ChunkRenderData&& crd);
    void renderDetector(const ChunkService& cs, size_t currentWorldID, Vec3d playerPosition);

    const World& mWorld;
    // Ranges
    int mRenderDist = 0;
    // Chunk Renderers
    std::unordered_map<Vec3i, ChunkRenderer> mChunkRenderers;
    
    bool neighbourChunkLoadCheck(const Vec3i& pos) const;
};



#endif // !WORLDCLIENT_H_
