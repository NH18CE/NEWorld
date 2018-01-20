// 
// NEWorld: Hitbox.cpp
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

#include "Hitbox.h"
#include <algorithm>

namespace Hitbox {
    bool inXclip(const AABB& boxA, const AABB& boxB) {
        if ((boxA.xmin > boxB.xmin && boxA.xmin < boxB.xmax) || (boxA.xmax > boxB.xmin && boxA.xmax < boxB.xmax))
            return
                true;
        if ((boxB.xmin > boxA.xmin && boxB.xmin < boxA.xmax) || (boxB.xmax > boxA.xmin && boxB.xmax < boxA.xmax))
            return
                true;
        return false;
    }

    bool inYclip(const AABB& boxA, const AABB& boxB) {
        if ((boxA.ymin > boxB.ymin && boxA.ymin < boxB.ymax) || (boxA.ymax > boxB.ymin && boxA.ymax < boxB.ymax))
            return
                true;
        if ((boxB.ymin > boxA.ymin && boxB.ymin < boxA.ymax) || (boxB.ymax > boxA.ymin && boxB.ymax < boxA.ymax))
            return
                true;
        return false;
    }

    bool inZclip(const AABB& boxA, const AABB& boxB) {
        if ((boxA.zmin > boxB.zmin && boxA.zmin < boxB.zmax) || (boxA.zmax > boxB.zmin && boxA.zmax < boxB.zmax))
            return
                true;
        if ((boxB.zmin > boxA.zmin && boxB.zmin < boxA.zmax) || (boxB.zmax > boxA.zmin && boxB.zmax < boxA.zmax))
            return
                true;
        return false;
    }

    bool hit(const AABB& boxA, const AABB& boxB) {
        return inXclip(boxA, boxB) && inYclip(boxA, boxB) && inZclip(boxA, boxB);
    }

    double maxMoveOnXclip(const AABB& boxA, const AABB& boxB, double movedist) {
        //用boxA去撞boxB，别搞反了
        if (!(inYclip(boxA, boxB) && inZclip(boxA, boxB))) return movedist;
        if (boxA.xmin >= boxB.xmax && movedist < 0.0) return std::max(boxB.xmax - boxA.xmin, movedist);
        if (boxA.xmax <= boxB.xmin && movedist > 0.0) return std::min(boxB.xmin - boxA.xmax, movedist);
        return movedist;
    }

    double maxMoveOnYclip(const AABB& boxA, const AABB& boxB, double movedist) {
        //用boxA去撞boxB，别搞反了 （这好像是句废话）
        if (!(inXclip(boxA, boxB) && inZclip(boxA, boxB))) return movedist;
        if (boxA.ymin >= boxB.ymax && movedist < 0.0) return std::max(boxB.ymax - boxA.ymin, movedist);
        if (boxA.ymax <= boxB.ymin && movedist > 0.0) return std::min(boxB.ymin - boxA.ymax, movedist);
        return movedist;
    }

    double maxMoveOnZclip(const AABB& boxA, const AABB& boxB, double movedist) {
        //用boxA去撞boxB，别搞反了 （这好像还是句废话）
        if (!(inXclip(boxA, boxB) && inYclip(boxA, boxB))) return movedist;
        if (boxA.zmin >= boxB.zmax && movedist < 0.0) return std::max(boxB.zmax - boxA.zmin, movedist);
        if (boxA.zmax <= boxB.zmin && movedist > 0.0) return std::min(boxB.zmin - boxA.zmax, movedist);
        return movedist;
    }

    AABB expand(const AABB& box, double xe, double ye, double ze) {
        AABB ret = box;
        if (xe > 0.0) ret.xmax += xe;
        else ret.xmin += xe;
        if (ye > 0.0) ret.ymax += ye;
        else ret.ymin += ye;
        if (ze > 0.0) ret.zmax += ze;
        else ret.zmin += ze;
        return ret;
    }

    void move(AABB& box, double xa, double ya, double za) {
        box.xmin += xa;
        box.ymin += ya;
        box.zmin += za;
        box.xmax += xa;
        box.ymax += ya;
        box.zmax += za;
    }

    void moveTo(AABB& box, double x, double y, double z) {
        //注意在执行这个过程时，参数中的xyz坐标将成为移动后的AABB的中心，而不是初始化AABB时的原点！
        const auto l = (box.xmax - box.xmin) / 2;
        const auto w = (box.ymax - box.ymin) / 2;
        const auto h = (box.zmax - box.zmin) / 2;
        box.xmin = x - l;
        box.ymin = y - w;
        box.zmin = z - h;
        box.xmax = x + l;
        box.ymax = y + w;
        box.zmax = z + h;
    }
}
