// 
// NEWorld: GameMenu.cpp
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
#include "Setup.h"

namespace Menus {
    class GameMenu : public GUI::Form {
    private:
        GUI::Label title;
        GUI::Button resumebtn, exitbtn;

        void onLoad() override {
            glfwSetInputMode(MainWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            title = GUI::Label(GetStrbyKey("NEWorld.pause.caption"), -225, 225, 0, 16, 0.5, 0.5, 0.25, 0.25);
            resumebtn = GUI::Button(GetStrbyKey("NEWorld.pause.continue"), -200, 200, -35, -3, 0.5, 0.5, 0.5, 0.5);
            exitbtn = GUI::Button(GetStrbyKey("NEWorld.pause.back"), -200, 200, 3, 35, 0.5, 0.5, 0.5, 0.5);
            registerControls({ &title, &resumebtn, &exitbtn });
        }

        void onUpdate() override {
            MutexUnlock(Mutex);
            //Make update thread realize that it should pause
            MutexLock(Mutex);
            if (resumebtn.clicked) {
                GUI::popPage();
                glfwSetInputMode(MainWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                glDepthFunc(GL_LEQUAL);
                glEnable(GL_CULL_FACE);
                setupNormalFog();
                double dmx, dmy;
                glfwGetCursorPos(MainWindow, &dmx, &dmy);
                mx = (int)(dmx / stretch), my = (int)(dmy / stretch);
                updateThreadPaused = false;
            }

        }
    };

    void gamemenu() { pushPage(new GameMenu); }
}
