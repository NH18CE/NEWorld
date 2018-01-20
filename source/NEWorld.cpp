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
#include "ModLoader.h"
#include "Setup.h"
#include <direct.h>
#include <iostream>
#include <fstream>

void loadOptions();
void saveOptions();

//==============================  Main Program  ================================//
//==============================     主程序     ================================//

void ApplicationBeforeLaunch() {
    loadOptions();
    Globalization::Load();

    _mkdir("Configs");
    _mkdir("Worlds");
    _mkdir("Screenshots");
    _mkdir("Mods");
}

void ApplicationAfterLaunch() {
    loadTextures();
    Module::ModLoader::loadMods();
    //初始化音频系统
    AudioSystem::Init();
}

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
    //反初始化音频系统
    AudioSystem::UnInit();
    return 0;
}

void AppCleanUp() {
    World::destroyAllChunks();
    Module::ModLoader::unloadMods();
}

template <typename T>
void loadoption(std::map<std::string, std::string>& m, const char* name, T& value) {
    if (m.find(name) == m.end()) return;
    std::stringstream ss;
    ss << m[name];
    ss >> value;
}

void loadOptions() {
    std::map<std::string, std::string> options;
    std::ifstream filein("Configs/options.ini", std::ios::in);
    if (!filein.is_open()) return;
    std::string name, value;
    while (!filein.eof()) {
        filein >> name >> value;
        options[name] = value;
    }
    filein.close();
    loadoption(options, "Language", Globalization::Cur_Lang);
    loadoption(options, "FOV", FOVyNormal);
    loadoption(options, "RenderDistance", viewdistance);
    loadoption(options, "Sensitivity", mousemove);
    loadoption(options, "CloudWidth", cloudwidth);
    loadoption(options, "SmoothLighting", SmoothLighting);
    loadoption(options, "FancyGrass", NiceGrass);
    loadoption(options, "MergeFaceRendering", MergeFace);
    loadoption(options, "MultiSample", Multisample);
    loadoption(options, "AdvancedRender", Renderer::AdvancedRender);
    loadoption(options, "ShadowMapRes", Renderer::ShadowRes);
    loadoption(options, "ShadowDistance", Renderer::maxShadowDist);
    loadoption(options, "VerticalSync", vsync);
    loadoption(options, "GUIBackgroundBlur", GUIScreenBlur);
    loadoption(options, "ppistretch", ppistretch);
    loadoption(options, "ForceUnicodeFont", TextRenderer::useUnicodeAsciiFont);
    loadoption(options, "GainOfBGM", AudioSystem::BGMGain);
    loadoption(options, "GainOfSound", AudioSystem::SoundGain);
}

template <typename T>
void saveoption(std::ofstream& out, const char* name, T& value) {
    out << std::string(name) << " " << value << std::endl;
}

void saveOptions() {
    std::map<std::string, std::string> options;
    std::ofstream fileout("Configs/options.ini", std::ios::out);
    if (!fileout.is_open()) return;
    saveoption(fileout, "Language", Globalization::Cur_Lang);
    saveoption(fileout, "FOV", FOVyNormal);
    saveoption(fileout, "RenderDistance", viewdistance);
    saveoption(fileout, "Sensitivity", mousemove);
    saveoption(fileout, "CloudWidth", cloudwidth);
    saveoption(fileout, "SmoothLighting", SmoothLighting);
    saveoption(fileout, "FancyGrass", NiceGrass);
    saveoption(fileout, "MergeFaceRendering", MergeFace);
    saveoption(fileout, "MultiSample", Multisample);
    saveoption(fileout, "AdvancedRender", Renderer::AdvancedRender);
    saveoption(fileout, "ShadowMapRes", Renderer::ShadowRes);
    saveoption(fileout, "ShadowDistance", Renderer::maxShadowDist);
    saveoption(fileout, "VerticalSync", vsync);
    saveoption(fileout, "GUIBackgroundBlur", GUIScreenBlur);
    saveoption(fileout, "ppistretch", ppistretch);
    saveoption(fileout, "ForceUnicodeFont", TextRenderer::useUnicodeAsciiFont);
    saveoption(fileout, "GainOfBGM", AudioSystem::BGMGain);
    saveoption(fileout, "GainOfSound", AudioSystem::SoundGain);
    fileout.close();
}
