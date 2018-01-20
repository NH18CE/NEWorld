// 
// NEWorld: RenderOptionsMenu.cpp
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
    class RenderOptionsMenu : public GUI::Form {
    private:
        GUI::Label title;
        GUI::Button smoothlightingbtn, fancygrassbtn, mergefacebtn, shaderbtn, vsyncbtn, backbtn;
        GUI::Trackbar msaabar;

        void onLoad() override {
            title = GUI::Label(GetStrbyKey("NEWorld.render.caption"), -225, 225, 20, 36, 0.5, 0.5, 0.0, 0.0);
            smoothlightingbtn = GUI::Button("", -250, -10, 60, 84, 0.5, 0.5, 0.0, 0.0);
            fancygrassbtn = GUI::Button("", 10, 250, 60, 84, 0.5, 0.5, 0.0, 0.0);
            mergefacebtn = GUI::Button("", -250, -10, 96, 120, 0.5, 0.5, 0.0, 0.0);
            msaabar = GUI::Trackbar("", 120, Multisample == 0 ? 0 : (int)(log2(Multisample) - 1) * 30 - 1, 10, 250, 96,
                                    120, 0.5, 0.5, 0.0, 0.0);
            shaderbtn = GUI::Button(GetStrbyKey("NEWorld.render.shaders"), -250, -10, 132, 156, 0.5, 0.5, 0.0, 0.0);
            vsyncbtn = GUI::Button("", 10, 250, 132, 156, 0.5, 0.5, 0.0, 0.0);
            backbtn = GUI::Button(GetStrbyKey("NEWorld.render.back"), -250, 250, -44, -20, 0.5, 0.5, 1.0, 1.0);
            registerControls({ &title, &smoothlightingbtn, &fancygrassbtn, &mergefacebtn, &msaabar, &shaderbtn,
                             &vsyncbtn, &backbtn });
        }

        void onUpdate() override {
            glfwGetMonitorPhysicalSize(glfwGetPrimaryMonitor(), &GUI::nScreenWidth,
                                       &GUI::nScreenHeight);
            if (smoothlightingbtn.clicked) SmoothLighting = !SmoothLighting;
            if (fancygrassbtn.clicked) NiceGrass = !NiceGrass;
            if (mergefacebtn.clicked) MergeFace = !MergeFace;
            if (msaabar.barpos == 0) Multisample = 0;
            else Multisample = 1 << ((msaabar.barpos + 1) / 30 + 1);
            if (shaderbtn.clicked) Shaderoptions();
            if (vsyncbtn.clicked) {
                vsync = !vsync;
                if (vsync) glfwSwapInterval(1);
                else glfwSwapInterval(0);
            }
            if (backbtn.clicked) GUI::popPage();
            std::stringstream ss;
            ss << Multisample;
            smoothlightingbtn.text = GetStrbyKey("NEWorld.render.smooth") + BoolEnabled(SmoothLighting);
            fancygrassbtn.text = GetStrbyKey("NEWorld.render.grasstex") + BoolYesNo(NiceGrass);
            mergefacebtn.text = GetStrbyKey("NEWorld.render.merge") + BoolEnabled(MergeFace);
            msaabar.text = GetStrbyKey("NEWorld.render.multisample") + (Multisample != 0
                                                                            ? ss.str() + "x"
                                                                            : BoolEnabled(false));
            vsyncbtn.text = GetStrbyKey("NEWorld.render.vsync") + BoolEnabled(vsync);

        }
    };

    void Renderoptions() { pushPage(new RenderOptionsMenu); }
}
