// 
// NWShared: Module.h
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
#include "Utils/Concept.h"
#include "Utils/Dylib.h"
#include "StdInclude.h"
#include <vector>

namespace Module {
    struct Version {
        int vMajor, vMinor, vRevision, vBuild;
        bool operator < (const Version& v) const noexcept {
            if (v.vMajor < vMajor) return true;
            else if (v.vMajor == vMajor && v.vMinor < vMinor) return true;
            else if (v.vMinor == vMinor && v.vRevision < vRevision) return true;
            else if (v.vRevision == vRevision && v.vBuild < vBuild) return true;
            return false;
        }
        bool operator == (const Version& v) const noexcept {
            return v.vMajor == vMajor && v.vMinor == vMinor && v.vRevision == vRevision && v.vBuild == vBuild;
        }
        bool operator > (const Version& v) const noexcept { return !(*this < v || *this == v); }
        bool operator >= (const Version& v) const noexcept { return (*this > v || *this == v); }
        bool operator <= (const Version& v) const noexcept { return (*this < v || *this == v); }
    };

    using VersionInfo = const std::pair<Version, Version>&;
    using DependencyInfo = const std::vector<std::pair<std::string, Version>>&;

    class NW_API Module final : Uncopyable {
    public:
        ~Module();
        Module(Module&&) = default;
        Module& operator = (Module&&) = default;
        void init();
        void preInit();
        VersionInfo getVersion() const;
        DependencyInfo getDependencies() const;
    private:
        friend class Server;
        Library mLibrary;
        explicit Module(const filesystem::path& pth);
    };

    class NW_API Server final : Singletion {
    public:
        ~Server();
        static Server& getInstance() noexcept;
    private:
        std::vector<Module> mModules;
        Server();
    };
}

