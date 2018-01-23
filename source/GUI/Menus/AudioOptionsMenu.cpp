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
    class AudioOptionsMenu final: public GUI::Form {
    private:
        GUI::Label mTitle;
        GUI::Button mBackBtn;
        GUI::Trackbar mBGMGain, mEffectGain,mGUIGain;

        void onLoad() override {
            mTitle = GUI::Label(I18N::get("NEWorld.sound.caption"), -225, 225, 20, 36, 0.5, 0.5, 0.0, 0.0);
            auto settings = getAudioSystem().getSettings();
            mBGMGain = GUI::Trackbar(I18N::get("NEWorld.sound.BGMGain"), 100, 
                static_cast<int>(settings.BGMGain * 300.0f), -200, 201,60, 84, 0.5, 0.5, 0.0, 0.0);
            mEffectGain = GUI::Trackbar(I18N::get("NEWorld.sound.effectGain"), 100, 
                static_cast<int>(settings.effectGain * 300.0f), -200,201, 90, 114, 0.5, 0.5, 0.0, 0.0);
            mGUIGain = GUI::Trackbar(I18N::get("NEWorld.sound.GUIGain"), 100,
                static_cast<int>(settings.GUIGain * 300.0f), -200,201, 120, 144, 0.5, 0.5, 0.0, 0.0);
            mBackBtn = GUI::Button(I18N::get("NEWorld.sound.back"), -250, 250, -44, -20, 0.5, 0.5, 1.0, 1.0);
            registerControls({ &mTitle, &mBGMGain, &mEffectGain, &mGUIGain,&mBackBtn });
        }

        void onUpdate() override {
            AudioSystemSettings settings;
            settings.BGMGain = mBGMGain.barpos / 300.0f;
            settings.effectGain = mEffectGain.barpos / 300.0f;
            settings.GUIGain = mGUIGain.barpos / 300.0f;
            getAudioSystem().setSettings(settings);
            mBGMGain.text = I18N::get("NEWorld.sound.BGMGain") +":"+std::to_string(mBGMGain.barpos/3)+"%";
            mEffectGain.text = I18N::get("NEWorld.sound.effectGain") +":"+ std::to_string(mEffectGain.barpos / 3) + "%";
            mGUIGain.text = I18N::get("NEWorld.sound.GUIGain") +":"+ std::to_string(mGUIGain.barpos / 3) + "%";
            if (mBackBtn.clicked) GUI::popPage();
        }
    };

    void audioOptionsMenu() { pushPage(new AudioOptionsMenu); }
}
