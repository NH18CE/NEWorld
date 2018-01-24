// 
// NEWorld: I18N.cpp
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

#include "StdInclude.h"
#include "I18N.h"
#include <fstream>
#include <ini.hpp>

namespace {
    std::map<std::string, std::string> kvs;

    I18N::LangInfo loadInfo(const std::string& kv) {
        I18N::LangInfo ret;
        std::ifstream file("Lang/" + kv + ".ini");
        if (file.good()) {
            INI::Parser x(file);
            ret.id = kv;
            ret.engId = x.top()("Info")["EngID"];
            ret.prettyName = x.top()("Info")["Pretty Name"];
        }
        return ret;
    }
}

std::string I18N::curLang = "zh_CN";

bool I18N::loadLang(const std::string& lang) {
    std::ifstream file("Lang/" + lang + ".ini");
    if (file.good()) {
        INI::Parser map(file);
        kvs = std::move(map.top()("Translations").values);
        return true;
    }
    return false;
}

bool I18N::load() { return loadLang(curLang); }

std::string I18N::get(const std::string& key) {
    const auto iter = kvs.find(key);
    if (iter != kvs.end())
        return iter->second;
    return key;
}

void I18N::iterate(std::function<void(const LangInfo&)> fcn) {
    for (auto& x : filesystem::directory_iterator("Lang"))
        if (x.path().extension().compare("ini")) {
            std::string name = x.path().filename().string();
            const auto info = loadInfo(name.substr(0, name.length() - 4));
            fcn(info);
        }
}
