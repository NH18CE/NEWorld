// 
// NEWorld: World.cpp
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

#include "World.h"
#include "Textures.h"
#include "Renderer.h"
#include "WorldGen.h"
#include "Particles.h"
#include <direct.h>
#include <algorithm>

namespace World {
    std::string worldname;

    Chunk** chunks;
    int loadedChunks, chunkArraySize;

    namespace ChunkPointerCache {
        namespace {
            thread_local Chunk* ptr = nullptr;
            thread_local ChunkID id = 0;
        }

        inline auto fetch(const ChunkID iid) noexcept { return (iid == id) ? ptr : nullptr; }

        inline auto write(const ChunkID iid, Chunk* iptr) noexcept {
            id = iid;
            ptr = iptr;
            return iptr;
        }
    }

    CPARegionalCache cpArray;
    HeightMap hMap;
    std::vector<unsigned int> vbuffersShouldDelete;

    void expandChunkArray(int cc);
    void reduceChunkArray(int cc);

    void init() {
        filesystem::create_directory("Worlds/" + worldname);
        filesystem::create_directory("Worlds/" + worldname + "/chunks");
        WorldGen::perlinNoiseInit(3404);
        cpArray.create((viewdistance + 2) * 2);
        hMap.setSize((viewdistance + 2) * 2 * 16);
        hMap.create();
    }

    inline std::pair<int, int> binarySearchChunks(const int len, const ChunkID cid) {
        int first = 0;
        int last = len - 1;
        int middle = (first + last) / 2;
        while (first <= last && chunks[middle]->id != cid) {
            if (chunks[middle]->id > cid) { last = middle - 1; }
            if (chunks[middle]->id < cid) { first = middle + 1; }
            middle = (first + last) / 2;
        }
        return std::make_pair(first, middle);
    }

    Chunk* addChunk(int x, int y, int z) {
        const auto cid = getChunkID(x, y, z); //Chunk ID
        const auto pos = binarySearchChunks(loadedChunks, cid);
        if (loadedChunks > 0 && chunks[pos.second]->id == cid) {
            printf("[Console][Error]");
            printf("Chunk(%d,%d,%d)has been loaded,when adding chunk.\n", x, y, z);
            return chunks[pos.second];
        }

        expandChunkArray(1);
        for (auto i = loadedChunks - 1; i >= pos.first + 1; i--) { chunks[i] = chunks[i - 1]; }
        chunks[pos.first] = new Chunk(x, y, z, cid);
        cpArray.AddChunk(chunks[pos.first], x, y, z);
        return ChunkPointerCache::write(cid, chunks[pos.first]);
    }

    void deleteChunk(int x, int y, int z) {
        const auto cid = getChunkID(x, y, z);
        if (const auto index = binarySearchChunks(loadedChunks, cid).second; chunks[index]->id == cid) {
            delete chunks[index];
            for (auto i = index; i < loadedChunks - 1; i++)
                chunks[i] = chunks[i + 1];
            cpArray.DeleteChunk(x, y, z);
            reduceChunkArray(1);
        }
    }

    Chunk* getChunkPtr(int x, int y, int z) {
        const auto cid = getChunkID(x, y, z);
        if (const auto ret = ChunkPointerCache::fetch(cid); ret) return ret;
        if (const auto ret = cpArray.get(x, y, z); ret) return ChunkPointerCache::write(cid, ret);
        if (loadedChunks > 0)
            if (const auto chk = chunks[binarySearchChunks(loadedChunks, cid).second]; chk->id == cid) {
                cpArray.set(x, y, z, chk);
                return ChunkPointerCache::write(cid, chk);
            }
        return nullptr;
    }

    void expandChunkArray(const int cc) {
        loadedChunks += cc;
        if (loadedChunks > chunkArraySize) {
            if (chunkArraySize < 1024) chunkArraySize = 1024;
            do chunkArraySize <<= 1;
            while (chunkArraySize < loadedChunks);
            if (const auto cp = static_cast<Chunk**>(realloc(chunks, chunkArraySize * sizeof(Chunk*))); cp &&
                loadedChunks != 0)
                chunks = cp;
            else if (loadedChunks) {
                destroyAllChunks();
                throw std::bad_alloc();
            }
        }
    }

    void reduceChunkArray(int cc) { loadedChunks -= cc; }

    std::vector<Hitbox::AABB> getHitboxes(const Hitbox::AABB& box) {
        //返回与box相交的所有方块AABB
        Hitbox::AABB blockbox;
        std::vector<Hitbox::AABB> res;
        for (int a = lround(box.xmin) - 1; a <= lround(box.xmax) + 1; a++) {
            for (int b = lround(box.ymin) - 1; b <= lround(box.ymax) + 1; b++) {
                for (int c = lround(box.zmin) - 1; c <= lround(box.zmax) + 1; c++) {
                    if (getBlockInfo(getBlock(a, b, c)).isSolid()) {
                        blockbox.xmin = a - 0.5;
                        blockbox.xmax = a + 0.5;
                        blockbox.ymin = b - 0.5;
                        blockbox.ymax = b + 0.5;
                        blockbox.zmin = c - 0.5;
                        blockbox.zmax = c + 0.5;
                        if (hit(box, blockbox)) res.push_back(blockbox);
                    }
                }
            }
        }
        return res;
    }

    bool inWater(const Hitbox::AABB& box) {
        Hitbox::AABB blockbox;
        for (int a = lround(box.xmin) - 1; a <= lround(box.xmax) + 1; a++) {
            for (int b = lround(box.ymin) - 1; b <= lround(box.ymax) + 1; b++) {
                for (int c = lround(box.zmin) - 1; c <= lround(box.zmax) + 1; c++) {
                    if (getBlock(a, b, c) == Blocks::WATER || getBlock(a, b, c) == Blocks::LAVA) {
                        blockbox.xmin = a - 0.5;
                        blockbox.xmax = a + 0.5;
                        blockbox.ymin = b - 0.5;
                        blockbox.ymax = b + 0.5;
                        blockbox.zmin = c - 0.5;
                        blockbox.zmax = c + 0.5;
                        if (hit(box, blockbox)) return true;
                    }
                }
            }
        }
        return false;
    }

    void updateblock(int x, int y, int z, bool blockchanged, int depth) {
        //Blockupdate

        if (depth > 4096) return;
        depth++;

        bool updated = blockchanged;
        int cx = getChunkPos(x);
        int cy = getChunkPos(y);
        int cz = getChunkPos(z);

        int bx = getBlockPos(x);
        int by = getBlockPos(y);
        int bz = getBlockPos(z);

        Chunk* cptr = getChunkPtr(cx, cy, cz);
        if (cptr != nullptr) {
            if (cptr == emptyChunkPtr) {
                cptr = addChunk(cx, cy, cz);
                cptr->load();
                cptr->mIsEmpty = false;
            }
            Brightness oldbrightness = cptr->getBrightness(bx, by, bz);
            bool skylighted = true;
            int yi = y + 1;
            int cyi = getChunkPos(yi);
            if (y < 0) skylighted = false;
            else {
                while (chunkLoaded(cx, cyi + 1, cz) && skylighted) {
                    if (getBlockInfo(getBlock(x, yi, z))
                        .
                        isOpaque() || getBlock(x, yi, z) == Blocks::WATER
                    ) { skylighted = false; }
                    yi++;
                    cyi = getChunkPos(yi);
                }
            }

            if (!getBlockInfo(getBlock(x, y, z))
                .
                isOpaque()
            ) {
                const std::array<Block, 7> blks = {
                    0,
                    getBlock(x, y, z + 1), //Front face
                    getBlock(x, y, z - 1), //Back face
                    getBlock(x + 1, y, z), //Right face
                    getBlock(x - 1, y, z), //Left face
                    getBlock(x, y + 1, z), //Top face
                    getBlock(x, y - 1, z)
                }; //Bottom face
                const std::array<Brightness, 7> brts = {
                    0,
                    getBrightness(x, y, z + 1), //Front face
                    getBrightness(x, y, z - 1), //Back face
                    getBrightness(x + 1, y, z), //Right face
                    getBrightness(x - 1, y, z), //Left face
                    getBrightness(x, y + 1, z), //Top face
                    getBrightness(x, y - 1, z)
                }; //Bottom face
                const int maxbrightness = std::max_element(brts.begin(), brts.end()) - brts.begin();
                Brightness br = brts[maxbrightness];
                if (blks[maxbrightness] == Blocks::WATER) br = std::max(Brightness(br - 2), BrightnessMin);
                if (skylighted && br < skylight) br = skylight;
                if (br < BrightnessMin) br = BrightnessMin;
                //Set brightness
                cptr->setbrightness(bx, by, bz, br);
            }
            else {
                //Opaque block
                cptr->setbrightness(bx, by, bz, 0);
                if (getBlock(x, y, z) == Blocks::GLOWSTONE || getBlock(x, y, z) == Blocks::LAVA) {
                    cptr->setbrightness(bx, by, bz, BrightnessMax);
                }
            }

            if (oldbrightness != cptr->getBrightness(bx, by, bz)) updated = true;

            if (updated) {
                updateblock(x, y + 1, z, false, depth);
                updateblock(x, y - 1, z, false, depth);
                updateblock(x + 1, y, z, false, depth);
                updateblock(x - 1, y, z, false, depth);
                updateblock(x, y, z + 1, false, depth);
                updateblock(x, y, z - 1, false, depth);
            }

            setChunkUpdated(cx, cy, cz, true);
            if (bx == 15) setChunkUpdated(cx + 1, cy, cz, true);
            if (bx == 0) setChunkUpdated(cx - 1, cy, cz, true);
            if (by == 15) setChunkUpdated(cx, cy + 1, cz, true);
            if (by == 0) setChunkUpdated(cx, cy - 1, cz, true);
            if (bz == 15) setChunkUpdated(cx, cy, cz + 1, true);
            if (bz == 0) setChunkUpdated(cx, cy, cz - 1, true);
        }
    }

    Block getBlock(int x, int y, int z, Block mask, Chunk* ci) {
        //获取方块
        int cx = getChunkPos(x), cy = getChunkPos(y), cz = getChunkPos(z);
        if (!ci || cx != ci->cx || cy != ci->cy || cz != ci->cz)
            ci = getChunkPtr(cx, cy, cz);
        if (ci == emptyChunkPtr) return Blocks::AIR;
        if (ci != nullptr) return ci->getBlock(getBlockPos(x), getBlockPos(y), getBlockPos(z));
        return mask;
    }

    Brightness getBrightness(int x, int y, int z, Chunk* ci) {
        //获取亮度
        int cx = getChunkPos(x), cy = getChunkPos(y), cz = getChunkPos(z);
        if (!ci || cx != ci->cx || cy != ci->cy || cz != ci->cz)
            ci = getChunkPtr(cx, cy, cz);
        if (ci == emptyChunkPtr) {
            if (cy < 0) return BrightnessMin;
            return skylight;
        }
        if (ci != nullptr)return ci->getBrightness(getBlockPos(x), getBlockPos(y), getBlockPos(z));
        return skylight;
    }

    void setblock(int x, int y, int z, Block Blockname, Chunk* cptr) {
        //设置方块
        int cx = getChunkPos(x), cy = getChunkPos(y), cz = getChunkPos(z);
        int bx = getBlockPos(x), by = getBlockPos(y), bz = getBlockPos(z);

        if (cptr != nullptr && cptr != emptyChunkPtr &&
            cx == cptr->cx && cy == cptr->cy && cz == cptr->cz) {
            cptr->setblock(bx, by, bz, Blockname);
            updateblock(x, y, z, true);
        }
        Chunk* i = getChunkPtr(cx, cy, cz);
        if (i == emptyChunkPtr) {
            Chunk* cp = addChunk(cx, cy, cz);
            cp->load();
            cp->mIsEmpty = false;
            i = cp;
        }
        if (i != nullptr) {
            i->setblock(bx, by, bz, Blockname);
            updateblock(x, y, z, true);
        }
    }

    void setbrightness(int x, int y, int z, Brightness Brightness, Chunk* cptr) {
        //设置亮度
        int cx = getChunkPos(x), cy = getChunkPos(y), cz = getChunkPos(z);
        int bx = getBlockPos(x), by = getBlockPos(y), bz = getBlockPos(z);

        if (cptr != nullptr && cptr != emptyChunkPtr &&
            cx == cptr->cx && cy == cptr->cy && cz == cptr->cz) { cptr->setbrightness(bx, by, bz, Brightness); }
        Chunk* i = getChunkPtr(cx, cy, cz);
        if (i == emptyChunkPtr) {
            Chunk* cp = addChunk(cx, cy, cz);
            cp->load();
            cp->mIsEmpty = false;
            i = cp;
        }
        if (i != nullptr) { i->setbrightness(bx, by, bz, Brightness); }
    }

    void picktree(int x, int y, int z) {
        if (getBlock(x, y, z) != Blocks::LEAF)Player::addItem(getBlock(x, y, z));
        else pickleaf();
        for (int j = 1; j <= 10; j++) {
            Particles::throwParticle(getBlock(x, y, z),
                                     float(x + rnd() - 0.5f), float(y + rnd() - 0.2f), float(z + rnd() - 0.5f),
                                     float(rnd() * 0.2f - 0.1f), float(rnd() * 0.2f - 0.1f), float(rnd() * 0.2f - 0.1f),
                                     float(rnd() * 0.02 + 0.03), int(rnd() * 60) + 30);
        }
        setblock(x, y, z, Blocks::AIR);
        //上
        if ((getBlock(x, y + 1, z) == Blocks::WOOD) || (getBlock(x, y + 1, z) == Blocks::LEAF))picktree(x, y + 1, z);
        //前
        if ((getBlock(x, y, z + 1) == Blocks::WOOD) || (getBlock(x, y, z + 1) == Blocks::LEAF))picktree(x, y, z + 1);
        //后
        if ((getBlock(x, y, z - 1) == Blocks::WOOD) || (getBlock(x, y, z - 1) == Blocks::LEAF))picktree(x, y, z - 1);
        //左
        if ((getBlock(x + 1, y, z) == Blocks::WOOD) || (getBlock(x + 1, y, z) == Blocks::LEAF))picktree(x + 1, y, z);
        //右
        if ((getBlock(x - 1, y, z) == Blocks::WOOD) || (getBlock(x - 1, y, z) == Blocks::LEAF))picktree(x - 1, y, z);
    }

    bool chunkUpdated(int x, int y, int z) {
        const auto i = getChunkPtr(x, y, z);
        return (i && i != emptyChunkPtr) ? i->mIsUpdated : false;
    }

    void setChunkUpdated(int x, int y, int z, bool value) {
        auto i = getChunkPtr(x, y, z);
        if (i == emptyChunkPtr) {
            auto cp = addChunk(x, y, z);
            cp->load();
            cp->mIsEmpty = false;
            i = cp;
        }
        if (i != nullptr) i->mIsUpdated = value;
    }

    void sortChunkBuildRenderList(int xpos, int ypos, int zpos) {
        int p = 0;

        int cxp = getChunkPos(xpos);
        int cyp = getChunkPos(ypos);
        int czp = getChunkPos(zpos);

        chunkBuildRenderList.clear();
        for (int ci = 0; ci < loadedChunks; ci++) {
            if (chunks[ci]->mIsUpdated) {
                int cx = chunks[ci]->cx;
                int cy = chunks[ci]->cy;
                int cz = chunks[ci]->cz;
                if (chunkInRange(cx, cy, cz, cxp, cyp, czp, viewdistance)) {
                    int xd = cx * 16 + 7 - xpos;
                    int yd = cy * 16 + 7 - ypos;
                    int zd = cz * 16 + 7 - zpos;
                    int distsqr = xd * xd + yd * yd + zd * zd;
                    chunkBuildRenderList.insert(distsqr, chunks[ci]);
                }
            }
        }
    }

    void sortChunkLoadUnloadList(int xpos, int ypos, int zpos) {
        int cx, cy, cz;
        int xd, yd, zd, distsqr;

        int cxp = getChunkPos(xpos);
        int cyp = getChunkPos(ypos);
        int czp = getChunkPos(zpos);

        chunkUnloadList.clear();
        for (int ci = 0; ci < loadedChunks; ci++) {
            cx = chunks[ci]->cx;
            cy = chunks[ci]->cy;
            cz = chunks[ci]->cz;
            if (!chunkInRange(cx, cy, cz, cxp, cyp, czp, viewdistance + 1)) {
                xd = cx * 16 + 7 - xpos;
                yd = cy * 16 + 7 - ypos;
                zd = cz * 16 + 7 - zpos;
                distsqr = xd * xd + yd * yd + zd * zd;
                chunkUnloadList.insert(distsqr, chunks[ci]);
            }
        }

        chunkLoadList.clear();
        for (cx = cxp - viewdistance - 1; cx <= cxp + viewdistance; cx++)
            for (cy = cyp - viewdistance - 1; cy <= cyp + viewdistance; cy++)
                for (cz = czp - viewdistance - 1; cz <= czp + viewdistance; cz++)
                    if (!cpArray.get(cx, cy, cz)) {
                        xd = cx * 16 + 7 - xpos;
                        yd = cy * 16 + 7 - ypos;
                        zd = cz * 16 + 7 - zpos;
                        distsqr = xd * xd + yd * yd + zd * zd;
                        chunkLoadList.insert(distsqr, {cx, cy, cz});
                    }
    }

    void calcVisible(double xpos, double ypos, double zpos, Frustum& frus) {
        Chunk::setRelativeBase(xpos, ypos, zpos, frus);
        for (int ci = 0; ci != loadedChunks; ci++) chunks[ci]->calcVisible();
    }

    void destroyAllChunks() {
        for (int i = 0; i != loadedChunks; i++)
            if (!chunks[i]->mIsEmpty)
                delete chunks[i];

        free(chunks);
        chunks = nullptr;
        loadedChunks = 0;
        chunkArraySize = 0;
        hMap.destroy();

        rebuiltChunks = 0;
        rebuiltChunksCount = 0;

        updatedChunks = 0;
        updatedChunksCount = 0;

        unloadedChunks = 0;
        unloadedChunksCount = 0;
    }

    void buildtree(int x, int y, int z) {
        //对生成条件进行更严格的检测
        //一：正上方五格必须为空气
        for (int i = y + 1; i < y + 6; i++)
            if (getBlock(x, i, z) != Blocks::AIR)return;
        //二：周围五格不能有树
        for (int ix = x - 4; ix < x + 4; ix++)
            for (int iy = y - 4; iy < y + 4; iy++)
                for (int iz = z - 4; iz < z + 4; iz++)
                    if (getBlock(ix, iy, iz) == Blocks::WOOD || getBlock(ix, iy, iz) == Blocks::LEAF)return;

        //终于可以开始生成了
        //设置泥土
        setblock(x, y, z, Blocks::DIRT);
        //设置树干
        int h = 0; //高度
        //测算泥土数量
        int Dirt = 0; //泥土数
        for (int ix = x - 4; ix < x + 4; ix++) {
            for (int iy = y - 4; iy < y; iy++) {
                for (int iz = z - 4; iz < z + 4; iz++) { if (getBlock(ix, iy, iz) == Blocks::DIRT)Dirt++; }
            }
        }
        //测算最高高度
        for (int i = y + 1; i < y + 16; i++) {
            if (getBlock(x, i, z) == Blocks::AIR) { h++; }
            else { break; }
        }
        //取最小值
        h = std::min(h, static_cast<int>(Dirt * 15 / 268 * 0.8));
        if (h < 7)return;
        //开始生成树干
        for (int i = y + 1; i < y + h + 1; i++) { setblock(x, i, z, Blocks::WOOD); }
        //设置树叶及枝杈
        //计算树叶起始生成高度
        int leafh = int(double(h) * 0.618) + 1; //黄金分割比大法好！！！
        int distancen2 = int(double((h - leafh + 1) * (h - leafh + 1))) + 1;
        for (int iy = y + leafh; iy < y + int(double(h) * 1.382) + 2; iy++) {
            for (int ix = x - 6; ix < x + 6; ix++) {
                for (int iz = z - 6; iz < z + 6; iz++) {
                    int distancen = Distancen(ix, iy, iz, x, y + leafh + 1, z);
                    if ((getBlock(ix, iy, iz) == Blocks::AIR) && (distancen < distancen2)) {
                        if ((distancen <= distancen2 / 9)) //生成枝杈
                        {
                            setblock(ix, iy, iz, Blocks::WOOD);
                        }
                        else //生成树叶
                        {
                            setblock(ix, iy, iz, Blocks::LEAF);
                        }
                    }
                }
            }
        }
    }

    void explode(int x, int y, int z, int r, Chunk* c) {
        double maxdistsqr = r * r;
        for (int fx = x - r - 1; fx < x + r + 1; fx++) {
            for (int fy = y - r - 1; fy < y + r + 1; fy++) {
                for (int fz = z - r - 1; fz < z + r + 1; fz++) {
                    int distsqr = (fx - x) * (fx - x) + (fy - y) * (fy - y) + (fz - z) * (fz - z);
                    if (distsqr <= maxdistsqr * 0.75 ||
                        (distsqr <= maxdistsqr && rnd() > (distsqr - maxdistsqr * 0.6) / (maxdistsqr * 0.4))) {
                        Block e = getBlock(fx, fy, fz);
                        if (e == Blocks::AIR) continue;
                        for (int j = 1; j <= 12; j++) {
                            Particles::throwParticle(e,
                                                     float(fx + rnd() - 0.5f), float(fy + rnd() - 0.2f),
                                                     float(fz + rnd() - 0.5f),
                                                     float(rnd() * 0.2f - 0.1f), float(rnd() * 0.2f - 0.1f),
                                                     float(rnd() * 0.2f - 0.1f),
                                                     float(rnd() * 0.02 + 0.03), int(rnd() * 60) + 30);
                        }
                        setblock(fx, fy, fz, Blocks::AIR, c);
                    }
                }
            }
        }
    }

    void randomChunkUpdation() noexcept {
        //随机状态更新
        for (int i = 0; i < World::loadedChunks; i++) {
            int cx = World::chunks[i]->cx;
            int cy = World::chunks[i]->cy;
            int cz = World::chunks[i]->cz;
            int x = int(rnd() * 16);
            int gx = x + cx * 16;
            int y = int(rnd() * 16);
            int gy = y + cy * 16;
            int z = int(rnd() * 16);
            int gz = z + cz * 16;
            if (World::chunks[i]->getBlock(x, y, z) == Blocks::DIRT &&
                World::getBlock(gx, gy + 1, gz, Blocks::NONEMPTY) == Blocks::AIR && (
                    World::getBlock(gx + 1, gy, gz, Blocks::AIR) == Blocks::GRASS ||
                    World::getBlock(gx - 1, gy, gz, Blocks::AIR) == Blocks::GRASS ||
                    World::getBlock(gx, gy, gz + 1, Blocks::AIR) == Blocks::GRASS ||
                    World::getBlock(gx, gy, gz - 1, Blocks::AIR) == Blocks::GRASS ||
                    World::getBlock(gx + 1, gy + 1, gz, Blocks::AIR) == Blocks::GRASS ||
                    World::getBlock(gx - 1, gy + 1, gz, Blocks::AIR) == Blocks::GRASS ||
                    World::getBlock(gx, gy + 1, gz + 1, Blocks::AIR) == Blocks::GRASS ||
                    World::getBlock(gx, gy + 1, gz - 1, Blocks::AIR) == Blocks::GRASS ||
                    World::getBlock(gx + 1, gy - 1, gz, Blocks::AIR) == Blocks::GRASS ||
                    World::getBlock(gx - 1, gy - 1, gz, Blocks::AIR) == Blocks::GRASS ||
                    World::getBlock(gx, gy - 1, gz + 1, Blocks::AIR) == Blocks::GRASS ||
                    World::getBlock(gx, gy - 1, gz - 1, Blocks::AIR) == Blocks::GRASS)) {
                //长草
                World::chunks[i]->setblock(x, y, z, Blocks::GRASS);
                World::updateblock(x + cx * 16, y + cy * 16 + 1, z + cz * 16, true);
                World::setChunkUpdated(cx, cy, cz, true);
            }
            if (World::chunks[i]->getBlock(x, y, z) == Blocks::GRASS && World::getBlock(gx, gy + 1, gz, Blocks::AIR) !=
                Blocks::AIR) {
                //草被覆盖
                World::chunks[i]->setblock(x, y, z, Blocks::DIRT);
                World::updateblock(x + cx * 16, y + cy * 16 + 1, z + cz * 16, true);
            }
        }
    }

    void updateChunkLoading() noexcept {
        World::sortChunkLoadUnloadList(lround(Player::xpos), lround(Player::ypos), lround(Player::zpos));

        //卸载区块(Unload chunks)
        for (auto&& [dist, cp] : World::chunkUnloadList)
            World::deleteChunk(cp->cx, cp->cy, cp->cz);

        //加载区块(Load chunks)
        for (auto&& [dist, pos] : World::chunkLoadList) {
            auto c = World::addChunk(pos.x, pos.y, pos.z);
            c->load(false);
            if (c->mIsEmpty) {
                World::deleteChunk(pos.x, pos.y, pos.z);
                World::cpArray.set(pos.x, pos.y, pos.z, World::emptyChunkPtr);
            }
        }
    }
}
