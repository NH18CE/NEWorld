// 
// NEWorld: Information.cpp
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
#include "TextRenderer.h"

std::string Str[] = {
    "NEWorld Main Version:" + MAJOR_VERSION + minOR_VERSION + EXT_VERSION,
    "CopyLeft 2016 Infinideastudio, No Rights Reserved",
    "Welcome to develope with us!",
    "Contributers:",
    "qiaozhanrong,Null,SuperSodaSea,Null,DREAMWORLDVOID,",
    "jacky8399,0u0,jelawat地鼠,HydroH,Michael R,dtcxzyw",
    "",
    "PS:Since this is a In-Develop Version, we welcome any types of suggestions or questions.",
    "Anyone is welcomed to send Issues on the following project site:",
    "https:\\\\github.com\\Infinideastudio\\NEWorld",
    "You can post bugs or request new features there",
    "If you have any problems, please contact us",
    "Thank you very much!"
};

namespace Menus {
    class Info : public GUI::Form {
    private:
        GUI::Label title;
        GUI::Button backbtn;

        void onLoad() override {
            backbtn = GUI::Button(GetStrbyKey("NEWorld.language.back"), -250, 250, -44, -20, 0.5, 0.5, 1.0, 1.0);
            registerControls({ &backbtn });
        }

        void onRender() override {
            for (int i = 0; i != 13; ++i) { TextRenderer::renderString(10, 10 + 20 * i, Str[i]); }
        }

        void onUpdate() override {
            AudioSystem::SpeedOfSound = AudioSystem::Air_SpeedOfSound;
            EFX::EAXprop = Generic;
            EFX::UpdateEAXprop();
            float Pos[] = {0.0f, 0.0f, 0.0f};
            AudioSystem::Update(Pos, false, false, Pos, false, false);
            if (backbtn.clicked) GUI::popPage();
        }
    };

    void Information() { pushPage(new Info); }
}
