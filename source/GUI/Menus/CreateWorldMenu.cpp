// 
// NEWorld: CreateWorldMenu.cpp
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
#include "GameView.h"

namespace Menus {
    class CreateWorldMenu : public GUI::Form {
    private:
        bool worldnametbChanged;
        GUI::Label title;
        GUI::Textbox worldnametb;
        GUI::Button okbtn, backbtn;

        void onLoad() override {
            title = GUI::Label(GetStrbyKey("NEWorld.create.caption"), -225, 225, 20, 36, 0.5, 0.5, 0.0, 0.0);
            worldnametb = GUI::Textbox(GetStrbyKey("NEWorld.create.inputname"), -250, 250, 48, 72, 0.5, 0.5, 0.0, 0.0);
            okbtn = GUI::Button(GetStrbyKey("NEWorld.create.ok"), -250, 250, 84, 120, 0.5, 0.5, 0.0, 0.0);
            backbtn = GUI::Button(GetStrbyKey("NEWorld.create.back"), -250, 250, -44, -20, 0.5, 0.5, 1.0, 1.0);
            registerControls({ &title, &worldnametb, &okbtn, &backbtn });
            inputstr = "";
            okbtn.enabled = false;
            worldnametbChanged = false;
        }

        void onUpdate() override {
            if (worldnametb.pressed && !worldnametbChanged) {
                worldnametb.text = "";
                worldnametbChanged = true;
            }
            if (worldnametb.text.empty() || !worldnametbChanged) okbtn.enabled = false;
            else okbtn.enabled = true;
            if (okbtn.clicked) {
                if (!worldnametb.text.empty()) {
                    World::worldname = worldnametb.text;
                    GUI::ClearStack();
                    GameView();
                }
                else { GUI::popPage(); }
            }
            AudioSystem::SpeedOfSound = AudioSystem::Air_SpeedOfSound;
            EFX::EAXprop = Generic;
            EFX::UpdateEAXprop();
            float Pos[] = {0.0f, 0.0f, 0.0f};
            AudioSystem::Update(Pos, false, false, Pos, false, false);
            if (backbtn.clicked) GUI::popPage();
            inputstr = "";
        }
    };

    void createworldmenu() { pushPage(new CreateWorldMenu); }
}
