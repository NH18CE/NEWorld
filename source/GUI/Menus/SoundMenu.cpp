// 
// NEWorld: SoundMenu.cpp
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
    class SoundMenu : public GUI::Form {
    private:
        GUI::Label title;
        GUI::Button backbtn;
        GUI::Trackbar Musicbar, SoundBar;

        void onLoad() override {
            title = GUI::Label(GetStrbyKey("NEWorld.Sound.caption"), -225, 225, 20, 36, 0.5, 0.5, 0.0, 0.0);
            Musicbar = GUI::Trackbar(GetStrbyKey("NEWorld.Sound.MusicGain"), 100, AudioSystem::BGMGain * 300, -200, 201,
                                     60, 84, 0.5, 0.5, 0.0, 0.0);
            SoundBar = GUI::Trackbar(GetStrbyKey("NEWorld.Sound.SoundGain"), 100, AudioSystem::SoundGain * 300, -200,
                                     201, 90, 114, 0.5, 0.5, 0.0, 0.0);
            backbtn = GUI::Button(GetStrbyKey("NEWorld.render.back"), -250, 250, -44, -20, 0.5, 0.5, 1.0, 1.0);
            registerControls({ &title, &Musicbar, &SoundBar, &backbtn });
        }

        void onUpdate() override {
            char text[100];
            AudioSystem::BGMGain = float(Musicbar.barpos) / 300.0f;
            AudioSystem::SoundGain = float(SoundBar.barpos) / 300.0f;
            sprintf_s(text, ":%d%%", Musicbar.barpos / 3);
            Musicbar.text = GetStrbyKey("NEWorld.Sound.MusicGain") + text;
            sprintf_s(text, ":%d%%", SoundBar.barpos / 3);
            SoundBar.text = GetStrbyKey("NEWorld.Sound.SoundGain") + text;
            AudioSystem::SpeedOfSound = AudioSystem::Air_SpeedOfSound;
            EFX::EAXprop = Generic;
            EFX::UpdateEAXprop();
            float Pos[] = {0.0f, 0.0f, 0.0f};
            AudioSystem::Update(Pos, false, false, Pos, false, false);
            if (backbtn.clicked) GUI::popPage();
        }
    };

    void Soundmenu() { pushPage(new SoundMenu); }
}
