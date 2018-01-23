// 
// NEWorld: ShaderOptionsMenu.cpp
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
#include "Renderer.h"

namespace Menus {
    class ShaderOptionsMenu : public GUI::Form {
    private:
        GUI::Label title;
        GUI::Button enablebtn, backbtn;
        GUI::Trackbar shadowresbar, shadowdistbar;

        void onLoad() override {
            title = GUI::Label(I18N::get("NEWorld.shaders.caption"), -225, 225, 20, 36, 0.5, 0.5, 0.0, 0.0);
            enablebtn = GUI::Button("", -250, -10, 60, 84, 0.5, 0.5, 0.0, 0.0);
            shadowresbar = GUI::Trackbar("", 120, (int)(log2(Renderer::ShadowRes) - 10) * 40 - 1, 10, 250, 60, 84, 0.5,
                                         0.5, 0.0, 0.0);
            shadowdistbar = GUI::Trackbar("", 120, (Renderer::maxShadowDist - 2) * 4 - 1, -250, -10, 96, 120, 0.5, 0.5,
                                          0.0, 0.0);
            backbtn = GUI::Button(I18N::get("NEWorld.render.back"), -250, 250, -44, -20, 0.5, 0.5, 1.0, 1.0);
            registerControls({ &title, &enablebtn, &shadowresbar, &shadowdistbar, &backbtn });
            Renderer::destroyShaders();
            if (!Renderer::AdvancedRender) shadowresbar.enabled = shadowdistbar.enabled = false;
        }

        void onUpdate() override {
            if (enablebtn.clicked) {
                Renderer::AdvancedRender = !Renderer::AdvancedRender;
                if (Renderer::AdvancedRender) shadowresbar.enabled = shadowdistbar.enabled = true;
                else shadowresbar.enabled = shadowdistbar.enabled = false;
            }
            Renderer::ShadowRes = (int)pow(2, (shadowresbar.barpos + 1) / 40 + 10);
            Renderer::maxShadowDist = (shadowdistbar.barpos + 1) / 4 + 2;
            if (backbtn.clicked) {
                GUI::popPage();
                if (Renderer::AdvancedRender) Renderer::initShaders();
            }
            enablebtn.text = I18N::get("NEWorld.shaders.enable") + BoolYesNo(Renderer::AdvancedRender);
            std::stringstream ss;
            ss << Renderer::ShadowRes;
            shadowresbar.text = I18N::get("NEWorld.shaders.shadowres") + ss.str() + "x";
            ss.str("");
            ss << Renderer::maxShadowDist;
            shadowdistbar.text = I18N::get("NEWorld.shaders.distance") + ss.str();

        }
    };

    void Shaderoptions() { pushPage(new ShaderOptionsMenu); }
}
