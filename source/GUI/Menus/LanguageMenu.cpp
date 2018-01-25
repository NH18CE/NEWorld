// 
// NEWorld: LanguageMenu.cpp
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
#include <deque>
#include <fstream>
#include <utility>

struct Langinfo {
    Langinfo(std::string str, GUI::Button* const btn) : symbol(std::move(str)), button(btn) {}
    std::string symbol;
    GUI::Button* button;
};

namespace Menus {
    class Language : public GUI::Form {
    private:
        std::deque<Langinfo> Langs;
        GUI::Label title;
        GUI::Button backbtn;

        void onLoad() override {
            Langs.clear();
            title = GUI::Label(I18N::get("NEWorld.language.caption"), -225, 225, 20, 36, 0.5, 0.5, 0.0, 0.0);
            backbtn = GUI::Button(I18N::get("NEWorld.language.back"), -250, 250, -44, -20, 0.5, 0.5, 1.0, 1.0);
            registerControls({&title, &backbtn});
            int count = 0;
            I18N::iterate([&count, this](const I18N::LangInfo& inf) {
                auto btn = new GUI::Button(inf.engId + "--" + inf.prettyName, -200, 200, count * 36 + 42,
                                           count * 36 + 72,
                                           0.5, 0.5, 0.0, 0.0);
                registerControl(btn);
                Langs.emplace_back(inf.id, btn);
                ++count;
            });
        }

        void onUpdate() override {
            if (backbtn.clicked) GUI::popPage();
            for (auto& lang : Langs) {
                if (lang.button->clicked) {
                    GUI::popPage();
                    if (I18N::getCurLang() != lang.symbol) {
                        I18N::loadLang(lang.symbol);
                        GUI::BackToMain();
                    }
                    break;
                }
            }
        }

        void onLeave() override {
            for (auto& lang : Langs) {
                for (auto iter = children.begin(); iter != children.end();) {
                    if ((*iter)->id == lang.button->id) iter = children.erase(iter);
                    else ++iter;
                }
                lang.button->destroy();
                delete lang.button;
            }
        }
    };

    void languagemenu() { pushPage(new Language); }
}
