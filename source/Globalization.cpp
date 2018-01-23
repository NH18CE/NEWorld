// 
// NEWorld: Globalization.cpp
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

#include "Globalization.h"
#include <fstream>
#include <ini.hpp>

namespace Globalization {
    int count;
    std::string Cur_Lang = "zh_CN";
    std::map<std::string, std::string> kvs;

    bool loadLang(const std::string& lang) {
        std::ifstream file("Lang/" + lang + ".ini");
        if (file.good()) {
            INI::Parser map(file);
            kvs = std::move(map.top()("Translations").values);
            return true;
        }
        return false;
    }

    bool load() {
        return loadLang(Cur_Lang);
    }

    std::string GetStrbyKey(const std::string& key) {
        const auto iter = kvs.find(key);
        if (iter != kvs.end())
            return iter->second;
        return key;
    }
}
