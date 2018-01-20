// 
// NEWorld: Hitbox.h
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

namespace Hitbox {
    //Axis Aligned Bounding Box
    struct AABB {
        double xmin, ymin, zmin;
        double xmax, ymax, zmax;
    };

    bool inXclip(const AABB& boxA, const AABB& boxB);
    bool inYclip(const AABB& boxA, const AABB& boxB);
    bool inZclip(const AABB& boxA, const AABB& boxB);

    bool hit(const AABB& boxA, const AABB& boxB);

    double maxMoveOnXclip(const AABB& boxA, const AABB& boxB, double movedist);
    double maxMoveOnYclip(const AABB& boxA, const AABB& boxB, double movedist);
    double maxMoveOnZclip(const AABB& boxA, const AABB& boxB, double movedist);

    AABB expand(const AABB& box, double xe, double ye, double ze);
    void move(AABB& box, double xa, double ya, double za);
    void moveTo(AABB& box, double x, double y, double z);
}
