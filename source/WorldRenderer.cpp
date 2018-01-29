// 
// NEWorld: WorldRenderer.cpp
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

#include "WorldRenderer.h"

namespace WorldRenderer {
    std::vector<RenderChunk> RenderChunkList;

    int ListRenderChunks(int cx, int cy, int cz, int renderdistance, double curtime, bool frustest) {
        int renderedChunks = 0;
        RenderChunkList.clear();
        for (auto& x : World::chunks) {
            if (!x->mIsRenderBuilt || x->mIsEmpty) continue;
            if (World::chunkInRange(x->cx, x->cy, x->cz, cx, cy, cz, renderdistance)) {
                if (!frustest || x->visible) {
                    renderedChunks++;
                    RenderChunkList.emplace_back(x, (curtime - lastupdate) * 30.0);
                }
            }
        }
        return renderedChunks;
    }

    void RenderChunks(double x, double y, double z, int buffer) {
        float m[16];
        if (buffer != 3) {
            memset(m, 0, sizeof(m));
            m[0] = m[5] = m[10] = m[15] = 1.0f;
        }

        for (auto cr : RenderChunkList) {
            if (cr.chk->vbo[buffer].isEmpty()) continue;
            glPushMatrix();
            glTranslated(cr.cx * 16.0 - x, cr.cy * 16.0 - cr.loadAnim - y, cr.cz * 16.0 - z);
            if (Renderer::AdvancedRender && buffer != 3) {
                m[12] = cr.cx * 16.0f - static_cast<float>(x);
                m[13] = cr.cy * 16.0f - static_cast<float>(cr.loadAnim) - static_cast<float>(y);
                m[14] = cr.cz * 16.0f - static_cast<float>(z);
                Renderer::shaders[Renderer::ActiveShader].setUniform("TransMat", m);
            }
            cr.chk->vbo[buffer].render();
            glPopMatrix();
        }

        glFlush();
    }
}
