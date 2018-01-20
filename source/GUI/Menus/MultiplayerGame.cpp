// 
// NEWorld: MultiplayerGame.cpp
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
    int getDotCount(const std::string& s) {
        int ret = 0;
        for (unsigned int i = 0; i != s.size(); i++)
            if (s[i] == '.') ret++;
        return ret;
    }

    class MultiplayerMenu : public GUI::Form {
    private:
        GUI::Label title;
        GUI::Textbox serveriptb;
        GUI::Button runbtn, okbtn, backbtn;

        void onLoad() override {
            title = GUI::Label("==============<  多 人 游 戏  >==============", -225, 225, 20, 36, 0.5, 0.5, 0.0, 0.0);
            serveriptb = GUI::Textbox("输入服务器IP", -225, 225, 20, 36, 0.5, 0.5, 0.0, 0.0);
            runbtn = GUI::Button("运行服务器（开服）", -225, 225, 20, 36, 0.5, 0.5, 0.0, 0.0);
            okbtn = GUI::Button("确定", -225, 225, 20, 36, 0.5, 0.5, 0.0, 0.0);
            backbtn = GUI::Button("<< 返回", -225, 225, 20, 36, 0.5, 0.5, 0.0, 0.0);
            inputstr = "";
            okbtn.enabled = false;
            registerControls({ &title, &serveriptb, &runbtn, &okbtn, &backbtn });
        }

        void onUpdate() override {
            static bool serveripChanged = false;
#ifdef NEWORLD_USE_WINAPI
            if (runbtn.clicked) WinExec("NEWorldServer.exe", SW_SHOWDEFAULT);
#endif
            if (okbtn.clicked) {
                serverip = serveriptb.text;
                gamebegin = true;
                multiplayer = true;
            }

            if (backbtn.clicked) GUI::popPage();
            if (serveriptb.pressed && !serveripChanged) {
                serveriptb.text = "";
                serveripChanged = true;
            }
            if (serveriptb.text.empty() || !serveripChanged || getDotCount(serveriptb.text) != 3) okbtn.enabled = false;
            else okbtn.enabled = true;
            inputstr = "";
        }
    };

    void multiplayermenu() { pushPage(new MultiplayerMenu); }
}
