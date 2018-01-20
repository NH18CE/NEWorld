// 
// NEWorld: WorldMenu.cpp
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

#include "Menus.h"
#include "World.h"
#include "Textures.h"
#include "GameView.h"
#include "TextRenderer.h"
#include <fstream>

namespace Menus {
    class WorldMenu : public GUI::Form {
    private:
        int leftp = static_cast<int>(windowwidth / 2.0 / stretch - 200);
        int midp = static_cast<int>(windowwidth / 2.0 / stretch);
        int rightp = static_cast<int>(windowwidth / 2.0 / stretch + 200);
        int downp = static_cast<int>(windowheight / stretch - 20);
        bool refresh = true;
        int selected = 0, mouseon;
        int worldcount;
        std::string chosenWorldName;
        std::vector<std::string> worldnames;
        std::vector<TextureID> thumbnails, texSizeX, texSizeY;
        int trs = 0;
        GUI::Label title;
        GUI::Vscroll vscroll;
        GUI::Button enterbtn, deletebtn, backbtn;

        void onLoad() override {
            title = GUI::Label(GetStrbyKey("NEWorld.worlds.caption"), -225, 225, 20, 36, 0.5, 0.5, 0.0, 0.0);
            vscroll = GUI::Vscroll(100, 0, 275, 295, 36, -20, 0.5, 0.5, 0.0, 1.0);
            enterbtn = GUI::Button(GetStrbyKey("NEWorld.worlds.enter"), -250, -10, -80, -56, 0.5, 0.5, 1.0, 1.0);
            deletebtn = GUI::Button(GetStrbyKey("NEWorld.worlds.delete"), 10, 250, -80, -56, 0.5, 0.5, 1.0, 1.0);
            backbtn = GUI::Button(GetStrbyKey("NEWorld.worlds.back"), -250, 250, -44, -20, 0.5, 0.5, 1.0, 1.0);
            registerControls({ &title, &vscroll, &enterbtn, &deletebtn, &backbtn });
            World::worldname = "";
            enterbtn.enabled = false;
            deletebtn.enabled = false;
            vscroll.defaultv = true;
        }

        void onUpdate() override {

            worldcount = (int)worldnames.size();
            leftp = static_cast<int>(windowwidth / 2.0 / stretch - 250);
            midp = static_cast<int>(windowwidth / 2.0 / stretch);
            rightp = static_cast<int>(windowwidth / 2.0 / stretch + 250);
            downp = static_cast<int>(windowheight / stretch - 20);

            vscroll.barheight = (downp - 72 - 48) * (downp - 36 - 40) / (64 * worldcount + 64);
            if (vscroll.barheight > downp - 36 - 40) {
                vscroll.enabled = false;
                vscroll.barheight = downp - 36 - 40;
            }
            else vscroll.enabled = true;

            trs = vscroll.barpos * (64 * worldcount + 64) / (downp - 36 - 40);
            mouseon = -1;
            if (mx >= midp - 250 && mx <= midp + 250 && my >= 48 && my <= downp - 72) {
                for (int i = 0; i < worldcount; i++) {
                    if (my >= 48 + i * 64 - trs && my <= 48 + i * 64 + 60 - trs) {
                        if (mb == 1 && mbl == 0) {
                            chosenWorldName = worldnames[i];
                            selected = i;
                        }
                        mouseon = i;
                    }
                }
                if (my >= 48 + worldcount * 64 - trs && my <= 48 + worldcount * 64 + 60 - trs) {
                    if (mb == 0 && mbl == 1) {
                        createworldmenu();
                        refresh = true;
                    }
                    mouseon = worldcount;
                }
            }
            if (enterbtn.clicked) {
                World::worldname = chosenWorldName;
                GUI::ClearStack();
                GameView();
            }
            if (deletebtn.clicked) {
                //删除世界文件
                filesystem::remove_all("Worlds/" + chosenWorldName);
                deletebtn.clicked = false;
                World::worldname.clear();
                enterbtn.enabled = false;
                deletebtn.enabled = false;
                refresh = true;
            }
            if (refresh) {
                worldnames.clear();
                thumbnails.clear();
                texSizeX.clear();
                texSizeY.clear();
                worldcount = 0;
                selected = -1;
                mouseon = -1;
                vscroll.barpos = 0;
                chosenWorldName = "";
                //查找所有世界存档
                Textures::TEXTURE_RGB tmb;
                for (auto&& x : filesystem::directory_iterator("Worlds/")) {
                    if (is_directory(x.path())) {
                        worldnames.push_back(x.path().filename().string());
                        const auto thumbName = x.path().string() + "/Thumbnail.bmp";
                        texSizeX.push_back(0);
                        texSizeY.push_back(0);
                        if (filesystem::exists(thumbName)) {
                            thumbnails.push_back(Textures::LoadRGBTexture(thumbName));
                            texSizeX[texSizeX.size() - 1] = tmb.sizeX;
                            texSizeY[texSizeY.size() - 1] = tmb.sizeY;
                        }
                        else
                            thumbnails.push_back(0);
                    }
                }
                refresh = false;
            }
            enterbtn.enabled = !chosenWorldName.empty();
            deletebtn.enabled = !chosenWorldName.empty();
            if (backbtn.clicked) GUI::popPage();
        }

        void onRender() override {
            glEnable(GL_SCISSOR_TEST);
            glScissor(0, windowheight - static_cast<int>((downp - 72) * stretch), windowwidth,
                      static_cast<int>((downp - 72 - 48 + 1) * stretch));
            glTranslatef(0.0f, (float)-trs, 0.0f);
            for (int i = 0; i < worldcount; i++) {
                int xmin, xmax, ymin, ymax;
                xmin = midp - 250, xmax = midp + 250;
                ymin = 48 + i * 64, ymax = 48 + i * 64 + 60;
                if (thumbnails[i] == -1) {
                    glDisable(GL_TEXTURE_2D);
                    if (mouseon == i) glColor4f(0.5, 0.5, 0.5, GUI::FgA);
                    else glColor4f(GUI::FgR, GUI::FgG, GUI::FgB, GUI::FgA);
                    glBegin(GL_QUADS);
                    UIVertex(midp - 250, 48 + i * 64);
                    UIVertex(midp + 250, 48 + i * 64);
                    UIVertex(midp + 250, 48 + i * 64 + 60);
                    UIVertex(midp - 250, 48 + i * 64 + 60);
                    glEnd();
                }
                else {
                    bool marginOnSides;
                    float w, h;
                    //计算材质坐标，保持高宽比（按钮大小为500x60），有小学数学基础的人仔细想一想应该能懂QAQ
                    if (texSizeX[i] * 60 / 500 < texSizeY[i]) {
                        marginOnSides = true;
                        w = 1.0f, h = texSizeX[i] * 60 / 500.0f / texSizeY[i];
                    }
                    else {
                        marginOnSides = false;
                        w = texSizeY[i] * 500 / 60.0f / texSizeX[i];
                        h = 1.0f;
                    }
                    glEnable(GL_TEXTURE_2D);
                    glBindTexture(GL_TEXTURE_2D, thumbnails[i]);
                    if (mouseon == (int)i) glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
                    else glColor4f(0.8f, 0.8f, 0.8f, 0.9f);
                    glBegin(GL_QUADS);
                    glTexCoord2f(0.5f - w / 2, 0.5f + h / 2), UIVertex(midp - 250, 48 + i * 64);
                    glTexCoord2f(0.5f + w / 2, 0.5f + h / 2), UIVertex(midp + 250, 48 + i * 64);
                    glTexCoord2f(0.5f + w / 2, 0.5f - h / 2), UIVertex(midp + 250, 48 + i * 64 + 60);
                    glTexCoord2f(0.5f - w / 2, 0.5f - h / 2), UIVertex(midp - 250, 48 + i * 64 + 60);
                    glEnd();
                }
                glColor4f(GUI::FgR * 0.9f, GUI::FgG * 0.9f, GUI::FgB * 0.9f, 0.9f);
                glDisable(GL_TEXTURE_2D);
                glLineWidth(1.0);
                glBegin(GL_LINE_LOOP);
                UIVertex(xmin, ymin);
                UIVertex(xmin, ymax);
                UIVertex(xmax, ymax);
                UIVertex(xmax, ymin);
                glEnd();
                if (selected == (int)i) {
                    glLineWidth(2.0);
                    glColor4f(0.0, 0.0, 0.0, 1.0);
                    glBegin(GL_LINE_LOOP);
                    UIVertex(midp - 250, 48 + i * 64);
                    UIVertex(midp + 250, 48 + i * 64);
                    UIVertex(midp + 250, 48 + i * 64 + 60);
                    UIVertex(midp - 250, 48 + i * 64 + 60);
                    glEnd();
                }
                TextRenderer::renderString(
                    static_cast<int>(windowwidth / stretch - TextRenderer::getStrWidth(worldnames[i])) / 2,
                    (140 + i * 128) / 2, worldnames[i]);
            }
            int i = worldcount;
            glDisable(GL_TEXTURE_2D);
            if (mouseon == i) glColor4f(0.5f, 0.5f, 0.5f, GUI::FgA);
            else glColor4f(GUI::FgR, GUI::FgG, GUI::FgB, GUI::FgA);
            glBegin(GL_QUADS);
            UIVertex(midp - 250, 48 + i * 64);
            UIVertex(midp + 250, 48 + i * 64);
            UIVertex(midp + 250, 48 + i * 64 + 60);
            UIVertex(midp - 250, 48 + i * 64 + 60);
            glEnd();
            glColor4f(GUI::FgR * 0.9f, GUI::FgG * 0.9f, GUI::FgB * 0.9f, 0.9f);
            glDisable(GL_TEXTURE_2D);
            glLineWidth(1.0);
            glBegin(GL_LINE_LOOP);
            UIVertex(midp - 250, 48 + i * 64);
            UIVertex(midp + 250, 48 + i * 64);
            UIVertex(midp + 250, 48 + i * 64 + 60);
            UIVertex(midp - 250, 48 + i * 64 + 60);
            glEnd();
            TextRenderer::renderString(
                static_cast<int>(windowwidth / stretch - TextRenderer::getStrWidth(GetStrbyKey("NEWorld.worlds.new"))) /
                2,
                (140 + i * 128) / 2, GetStrbyKey("NEWorld.worlds.new"));
            glDisable(GL_SCISSOR_TEST);
        }
    };

    void worldmenu() { pushPage(new WorldMenu); }
}
