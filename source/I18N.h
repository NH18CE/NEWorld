// 
// NWShared: I18N.h
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

#pragma once

#include "Config/Config.hpp"
#include <string>
#include <functional>

namespace I18N {
    struct LangInfo {
        std::string id, engId, prettyName;
    };

    NW_API bool load();
    NW_API bool loadLang(const std::string& lang);
    NW_API void setCurLang(std::string nv);
    NW_API std::string getCurLang() noexcept;
    NW_API std::string get(const std::string& key);
    NW_API void iterate(std::function<void(const LangInfo&)> fcn);
};
