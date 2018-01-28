// 
// NEWorld: NEWorld.cpp
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

#include "Definitions.h"
#include "Renderer.h"
#include "TextRenderer.h"
#include "World.h"
#include "Menus.h"
#include "Setup.h"
#include <iostream>
#include <fstream>
#include <ini.hpp>

void loadOptions();
void saveOptions();

//==============================  Main Program  ================================//
//==============================     主程序     ================================//

void ApplicationBeforeLaunch() {
    loadOptions();

    filesystem::create_directory("Configs");
    filesystem::create_directory("Worlds");
    filesystem::create_directory("Screenshots");
    filesystem::create_directory("Mods");
}

void ApplicationAfterLaunch() { loadTextures(); }

int main() {
    ApplicationBeforeLaunch();
    windowwidth = defaultwindowwidth;
    windowheight = defaultwindowheight;
    std::cout << "[Console][Event]Initialize GLFW" << (glfwInit() == 1 ? "" : " - Failed!") << std::endl;
    createWindow();
    setupScreen();
    glDisable(GL_CULL_FACE);
    splashScreen();
    ApplicationAfterLaunch();
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glDisable(GL_LINE_SMOOTH);
    GUI::clearTransition();
    //App Entrance
    GUI::BackToMain();
    GUI::appStart();
    //结束程序，删了也没关系 ←_←（吐槽FB和glfw中）
    //不对啊这不是FB！！！这是正宗的C++！！！！！！
    //楼上的楼上在瞎说！！！别信他的！！！
    //……所以你是不是应该说“吐槽C艹”中？——地鼠
    glfwTerminate();
    return 0;
}

void AppCleanUp() { World::destroyAllChunks(); }

template <typename T>
void loadoption(INI::Parser& options, const char* section,
                const char* name, T& value) {
    std::stringstream ss;
    ss << options.top()(section)[name];
    ss >> value;
}

void loadOptions() {
    try {
        INI::Parser options("Configs/options.ini");
        {
            std::string curLang;
            loadoption(options, "Localization", "Language", curLang);
            I18N::setCurLang(std::move(curLang));
        }
        loadoption(options, "Rendering", "FOV", FOVyNormal);
        loadoption(options, "Rendering", "RenderDistance", viewdistance);
        loadoption(options, "Rendering", "Sensitivity", mousemove);
        loadoption(options, "Rendering", "CloudWidth", cloudwidth);
        loadoption(options, "Rendering", "SmoothLighting", SmoothLighting);
        loadoption(options, "Rendering", "FancyGrass", NiceGrass);
        loadoption(options, "Rendering", "MergeFaceRendering", MergeFace);
        loadoption(options, "Rendering", "MultiSample", Multisample);
        loadoption(options, "Rendering", "AdvancedRender", Renderer::AdvancedRender);
        loadoption(options, "Shading", "ShadowMapRes", Renderer::ShadowRes);
        loadoption(options, "Shading", "ShadowDistance", Renderer::maxShadowDist);
        loadoption(options, "Rendering", "VerticalSync", vsync);
        loadoption(options, "GUI", "GUIBackgroundBlur", GUIScreenBlur);
        loadoption(options, "GUI", "ppistretch", ppistretch);
        loadoption(options, "GUI", "ForceUnicodeFont", TextRenderer::useUnicodeAsciiFont);
        AudioSystemSettings settings;
        loadoption(options, "Audio", "GainOfBGM", settings.BGMGain);
        loadoption(options, "Audio", "GainOfEffect", settings.effectGain);
        loadoption(options, "Audio", "GainOfGUI", settings.GUIGain);
        getAudioSystem().setSettings(settings);
    }
    catch(...) {}
}

template <typename T>
void saveoption(INI::Parser& parser, const char* section, const char* name, const T& value) {
    std::stringstream ss;
    ss << value;
    parser.top()(section).values[name] = ss.str();
}

void saveOptions() {
    INI::Parser options{};
    options.top() = {};
    saveoption(options, "Localization", "Language", std::string(I18N::getCurLang()));
    saveoption(options, "Rendering", "FOV", FOVyNormal);
    saveoption(options, "Rendering", "RenderDistance", viewdistance);
    saveoption(options, "Rendering", "Sensitivity", mousemove);
    saveoption(options, "Rendering", "CloudWidth", cloudwidth);
    saveoption(options, "Rendering", "SmoothLighting", SmoothLighting);
    saveoption(options, "Rendering", "FancyGrass", NiceGrass);
    saveoption(options, "Rendering", "MergeFaceRendering", MergeFace);
    saveoption(options, "Rendering", "MultiSample", Multisample);
    saveoption(options, "Rendering", "AdvancedRender", Renderer::AdvancedRender);
    saveoption(options, "Shading", "ShadowMapRes", Renderer::ShadowRes);
    saveoption(options, "Shading", "ShadowDistance", Renderer::maxShadowDist);
    saveoption(options, "Rendering", "VerticalSync", vsync);
    saveoption(options, "GUI", "GUIBackgroundBlur", GUIScreenBlur);
    saveoption(options, "GUI", "ppistretch", ppistretch);
    saveoption(options, "GUI", "ForceUnicodeFont", TextRenderer::useUnicodeAsciiFont);
    auto settings = getAudioSystem().getSettings();
    saveoption(options, "Audio", "GainOfBGM", settings.BGMGain);
    saveoption(options, "Audio", "GainOfEffect", settings.effectGain);
    saveoption(options, "Audio", "GainOfGUI", settings.GUIGain);
    std::ofstream fileout("Configs/options.ini", std::ios::out);
    if (!fileout.is_open()) return;
    auto&& top = options.top();
    top.depth = 0;
    for (auto it = top.sections.cbegin(); it != top.sections.cend(); ++it)
        top.ordered_sections.push_back(it);
    for (auto&& sections : top.sections) {
        sections.second.depth = 1;
        auto&& values = sections.second.values;
        for (auto it = values.cbegin(); it != values.cend(); ++it)
            sections.second.ordered_values.push_back(it);
    }
    options.dump(fileout);
    fileout.close();
}
