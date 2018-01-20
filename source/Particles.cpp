// 
// NEWorld: Particles.cpp
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

#include "Particles.h"
#include "World.h"
#include "Textures.h"

namespace Particles {
    std::vector<Particle> ptcs;
    int ptcsrendered;
    double pxpos, pypos, pzpos;

    void update(Particle& ptc) {
        double dx, dy, dz;
        float psz = ptc.psize;

        ptc.hb.xmin = ptc.xpos - psz;
        ptc.hb.xmax = ptc.xpos + psz;
        ptc.hb.ymin = ptc.ypos - psz;
        ptc.hb.ymax = ptc.ypos + psz;
        ptc.hb.zmin = ptc.zpos - psz;
        ptc.hb.zmax = ptc.zpos + psz;

        dx = ptc.xsp;
        dy = ptc.ysp;
        dz = ptc.zsp;

        std::vector<Hitbox::AABB> Hitboxes = World::getHitboxes(expand(ptc.hb, dx, dy, dz));
        int hitnum = Hitboxes.size();
        for (int i = 0; i < hitnum; i++) { dy = maxMoveOnYclip(ptc.hb, Hitboxes[i], dy); }
        move(ptc.hb, 0.0, dy, 0.0);
        for (int i = 0; i < hitnum; i++) { dx = maxMoveOnXclip(ptc.hb, Hitboxes[i], dx); }
        move(ptc.hb, dx, 0.0, 0.0);
        for (int i = 0; i < hitnum; i++) { dz = maxMoveOnZclip(ptc.hb, Hitboxes[i], dz); }
        move(ptc.hb, 0.0, 0.0, dz);

        ptc.xpos += dx;
        ptc.ypos += dy;
        ptc.zpos += dz;
        if (dy != ptc.ysp) ptc.ysp = 0.0;
        ptc.xsp *= 0.6f;
        ptc.zsp *= 0.6f;
        ptc.ysp -= 0.01f;
        ptc.lasts -= 1;
    }

    void updateall() {
        for (std::vector<Particle>::iterator iter = ptcs.begin(); iter < ptcs.end();) {
            if (!iter->exist) continue;
            update(*iter);
            if (iter->lasts <= 0) {
                iter->exist = false;
                iter = ptcs.erase(iter);
            }
            else { ++iter; }
        }
    }

    void render(Particle& ptc) {
        //if (!Frustum::aabbInFrustum(ptc.hb)) return;
        ptcsrendered++;
        float size = (float)BLOCKTEXTURE_UNITSIZE / BLOCKTEXTURE_SIZE * (ptc.psize <= 1.0f ? ptc.psize : 1.0f);
        float col = World::getBrightness(lround(ptc.xpos), lround(ptc.ypos), lround(ptc.zpos)) / (float)World::
            BrightnessMax;
        float col1 = col * 0.5f;
        float col2 = col * 0.7f;
        float tcx = ptc.tcX;
        float tcy = ptc.tcY;
        float psize = ptc.psize;
        double palpha = (ptc.lasts < 30 ? ptc.lasts / 30.0 : 1.0);
        double xpos = ptc.xpos - pxpos;
        double ypos = ptc.ypos - pypos;
        double zpos = ptc.zpos - pzpos;

        glBegin(GL_QUADS);
        glColor4d(col1, col1, col1, palpha);
        glTexCoord2d(tcx + size * 0.0, tcy + size * 0.0);
        glVertex3d(xpos - psize, ypos - psize, zpos + psize);
        glTexCoord2d(tcx + size * 1.0, tcy + size * 0.0);
        glVertex3d(xpos + psize, ypos - psize, zpos + psize);
        glTexCoord2d(tcx + size * 1.0, tcy + size * 1.0);
        glVertex3d(xpos + psize, ypos + psize, zpos + psize);
        glTexCoord2d(tcx + size * 0.0, tcy + size * 1.0);
        glVertex3d(xpos - psize, ypos + psize, zpos + psize);

        glColor4d(col1, col1, col1, palpha);
        glTexCoord2d(tcx + size * 0.0, tcy + size * 0.0);
        glVertex3d(xpos - psize, ypos + psize, zpos - psize);
        glTexCoord2d(tcx + size * 1.0, tcy + size * 0.0);
        glVertex3d(xpos + psize, ypos + psize, zpos - psize);
        glTexCoord2d(tcx + size * 1.0, tcy + size * 1.0);
        glVertex3d(xpos + psize, ypos - psize, zpos - psize);
        glTexCoord2d(tcx + size * 0.0, tcy + size * 1.0);
        glVertex3d(xpos - psize, ypos - psize, zpos - psize);

        glColor4d(col, col, col, palpha);
        glTexCoord2d(tcx + size * 0.0, tcy + size * 0.0);
        glVertex3d(xpos + psize, ypos + psize, zpos - psize);
        glTexCoord2d(tcx + size * 1.0, tcy + size * 0.0);
        glVertex3d(xpos - psize, ypos + psize, zpos - psize);
        glTexCoord2d(tcx + size * 1.0, tcy + size * 1.0);
        glVertex3d(xpos - psize, ypos + psize, zpos + psize);
        glTexCoord2d(tcx + size * 0.0, tcy + size * 1.0);
        glVertex3d(xpos + psize, ypos + psize, zpos + psize);

        glColor4d(col, col, col, palpha);
        glTexCoord2d(tcx + size * 0.0, tcy + size * 0.0);
        glVertex3d(xpos - psize, ypos - psize, zpos - psize);
        glTexCoord2d(tcx + size * 1.0, tcy + size * 0.0);
        glVertex3d(xpos + psize, ypos - psize, zpos - psize);
        glTexCoord2d(tcx + size * 1.0, tcy + size * 1.0);
        glVertex3d(xpos + psize, ypos - psize, zpos + psize);
        glTexCoord2d(tcx + size * 0.0, tcy + size * 1.0);
        glVertex3d(xpos - psize, ypos - psize, zpos + psize);

        glColor4d(col2, col2, col2, palpha);
        glTexCoord2d(tcx + size * 0.0, tcy + size * 0.0);
        glVertex3d(xpos + psize, ypos + psize, zpos - psize);
        glTexCoord2d(tcx + size * 1.0, tcy + size * 0.0);
        glVertex3d(xpos + psize, ypos + psize, zpos + psize);
        glTexCoord2d(tcx + size * 1.0, tcy + size * 1.0);
        glVertex3d(xpos + psize, ypos - psize, zpos + psize);
        glTexCoord2d(tcx + size * 0.0, tcy + size * 1.0);
        glVertex3d(xpos + psize, ypos - psize, zpos - psize);

        glColor4d(col2, col2, col2, palpha);
        glTexCoord2d(tcx + size * 0.0, tcy + size * 0.0);
        glVertex3d(xpos - psize, ypos - psize, zpos - psize);
        glTexCoord2d(tcx + size * 1.0, tcy + size * 0.0);
        glVertex3d(xpos - psize, ypos - psize, zpos + psize);
        glTexCoord2d(tcx + size * 1.0, tcy + size * 1.0);
        glVertex3d(xpos - psize, ypos + psize, zpos + psize);
        glTexCoord2d(tcx + size * 0.0, tcy + size * 1.0);
        glVertex3d(xpos - psize, ypos + psize, zpos - psize);
        glEnd();
    }

    void renderall(double xpos, double ypos, double zpos) {
        pxpos = xpos;
        pypos = ypos;
        pzpos = zpos;
        ptcsrendered = 0;
        for (unsigned int i = 0; i != ptcs.size(); i++) {
            if (!ptcs[i].exist) continue;
            render(ptcs[i]);
        }
    }

    void throwParticle(Block pt, float x, float y, float z, float xs, float ys, float zs, float psz, int last) {
        float tcX1 = (float)Textures::getTexcoordX(pt, 2);
        float tcY1 = (float)Textures::getTexcoordY(pt, 2);
        Particle ptc;
        ptc.exist = true;
        ptc.xpos = x;
        ptc.ypos = y;
        ptc.zpos = z;
        ptc.xsp = xs;
        ptc.ysp = ys;
        ptc.zsp = zs;
        ptc.psize = psz;
        ptc.hb.xmin = x - psz;
        ptc.hb.xmax = x + psz;
        ptc.hb.ymin = y - psz;
        ptc.hb.ymax = y + psz;
        ptc.hb.zmin = z - psz;
        ptc.hb.zmax = z + psz;
        ptc.lasts = last;
        ptc.tcX = tcX1 + (float)rnd() * ((float)BLOCKTEXTURE_UNITSIZE / BLOCKTEXTURE_SIZE) * (1.0f - psz);
        ptc.tcY = tcY1 + (float)rnd() * ((float)BLOCKTEXTURE_UNITSIZE / BLOCKTEXTURE_SIZE) * (1.0f - psz);
        ptcs.push_back(ptc);
    }
}
