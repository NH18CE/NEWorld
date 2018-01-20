// 
// NEWorld: GUIOptionsMenu.cpp
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

namespace Menus {
    class GUIOptionsMenu : public GUI::Form {
    private:
        GUI::Label title, ppistat;
        GUI::Button fontbtn, blurbtn, ppistretchbtn, backbtn;

        void onLoad() override {
            title = GUI::Label(GetStrbyKey("NEWorld.gui.caption"), -225, 225, 20, 36, 0.5, 0.5, 0.0, 0.0);
            fontbtn = GUI::Button("", -250, -10, 60, 84, 0.5, 0.5, 0.0, 0.0);
            blurbtn = GUI::Button("", 10, 250, 60, 84, 0.5, 0.5, 0.0, 0.0);
            ppistretchbtn = GUI::Button(GetStrbyKey("NEWorld.gui.stretch"), -250, -10, 96, 120, 0.5, 0.5, 0.0, 0.0);
            ppistat = GUI::Label("", -250, 250, 120, 144, 0.5, 0.5, 0.0, 0.0);
            backbtn = GUI::Button(GetStrbyKey("NEWorld.gui.back"), -250, 250, -44, -20, 0.5, 0.5, 1.0, 1.0);
            registerControls({ &title, &fontbtn, &blurbtn, &ppistretchbtn, &ppistat, &backbtn });
            fontbtn.enabled = false;
        }

        void onUpdate() override {
            //if (fontbtn.clicked) TextRenderer::useUnicodeASCIIFont = !TextRenderer::useUnicodeASCIIFont;
            if (blurbtn.clicked) GUIScreenBlur = !GUIScreenBlur;
            if (ppistretchbtn.clicked) {
                if (stretch == 1.0) GUI::initStretch();
                else GUI::endStretch();
            }
            AudioSystem::SpeedOfSound = AudioSystem::Air_SpeedOfSound;
            EFX::EAXprop = Generic;
            EFX::UpdateEAXprop();
            float Pos[] = {0.0f, 0.0f, 0.0f};
            AudioSystem::Update(Pos, false, false, Pos, false, false);
            if (backbtn.clicked) GUI::popPage();
            //fontbtn.text = GetStrbyKey("NEWorld.gui.unicode") + BoolYesNo(TextRenderer::useUnicodeASCIIFont);
            fontbtn.text = GetStrbyKey("NEWorld.gui.unicode") + BoolYesNo(true);
            blurbtn.text = GetStrbyKey("NEWorld.gui.blur") + BoolEnabled(GUIScreenBlur);
            int vmc;
            const GLFWvidmode* m = glfwGetVideoModes(glfwGetPrimaryMonitor(), &vmc);
            ppistat.text = "phy:" + Var2Str(GUI::nScreenWidth) + "x" + Var2Str(GUI::nScreenHeight) +
                " scr:" + Var2Str(m[vmc - 1].width) + "x" + Var2Str(m[vmc - 1].height) +
                " win:" + Var2Str(windowwidth) + "x" + Var2Str(windowheight);
        }
    };

    void GUIoptions() { pushPage(new GUIOptionsMenu); }
}
