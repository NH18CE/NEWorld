// 
// NEWorld: Blocks.h
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
#include <utility>
#include "StdInclude.h"
#include "Definitions.h"
#include "I18N.h"

namespace Blocks {
    enum BlockID {
        AIR,
        ROCK,
        GRASS,
        DIRT,
        STONE,
        PLANK,
        WOOD,
        BEDROCK,
        LEAF,
        GLASS,
        WATER,
        LAVA,
        GLOWSTONE,
        SAND,
        CEMENT,
        ICE,
        COAL,
        IRON,
        TNT,
        BLOCK_DEF_END
    };

    const Block NONEMPTY = 1;

    class SingleBlock {
    public:
        SingleBlock(std::string blockName, bool solid, bool opaque, bool translucent, bool canExplode,
                    float hardness) :
            mUri(std::move(blockName)), mHardness(hardness), mSolid(solid), mOpaque(opaque), mTranslucent(translucent),
            mIsDark(false), mCanExplode(canExplode) { };

        //获得方块名称
        std::string getBlockName() const { return I18N::get(mUri); }
        //是否是固体
        bool isSolid() const { return mSolid; }
        //是否不透明
        bool isOpaque() const { return mOpaque; }
        //是否半透明
        bool isTranslucent() const { return mTranslucent; }
        //是否可以爆炸
        bool canExplode() const { return mCanExplode; }
        //获得硬度（数值越大硬度越小，最大100）
        float getHardness() const { return mHardness; }
    private:
        std::string mUri;
        float mHardness;
        bool mSolid, mOpaque, mTranslucent, mIsDark, mCanExplode;
    };

    const SingleBlock blockData[BLOCK_DEF_END + 1] = {
        //            方块名称          固体     不透明      半透明  可以爆炸  硬度
        SingleBlock("NEWorld.Blocks.Air", false, false, false, false, 0),
        SingleBlock("NEWorld.Blocks.Rock", true, true, false, false, 2),
        SingleBlock("NEWorld.Blocks.Grass", true, true, false, false, 5),
        SingleBlock("NEWorld.Blocks.Dirt", true, true, false, false, 5),
        SingleBlock("NEWorld.Blocks.Stone", true, true, false, false, 2),
        SingleBlock("NEWorld.Blocks.Plank", true, true, false, false, 5),
        SingleBlock("NEWorld.Blocks.Wood", true, true, false, false, 5),
        SingleBlock("NEWorld.Blocks.Bedrock", true, true, false, false, 0),
        SingleBlock("NEWorld.Blocks.Leaf", true, false, false, false, 15),
        SingleBlock("NEWorld.Blocks.Glass", true, false, false, false, 30),
        SingleBlock("NEWorld.Blocks.Water", false, false, true, false, 0),
        SingleBlock("NEWorld.Blocks.Lava", false, false, true, false, 0),
        SingleBlock("NEWorld.Blocks.GlowStone", true, true, false, false, 10),
        SingleBlock("NEWorld.Blocks.Sand", true, true, false, false, 8),
        SingleBlock("NEWorld.Blocks.Cement", true, true, false, false, 0.5f),
        SingleBlock("NEWorld.Blocks.Ice", true, false, true, false, 25),
        SingleBlock("NEWorld.Blocks.Coal Block", true, true, false, false, 1),
        SingleBlock("NEWorld.Blocks.Iron Block", true, true, false, false, 0.5f),
        SingleBlock("NEWorld.Blocks.TNT", true, true, false, true, 25),
        SingleBlock("NEWorld.Blocks.Null Block", true, true, false, false, 0)
    };
}

inline auto getBlockInfo(Block blockId) noexcept { return Blocks::blockData[blockId]; }
