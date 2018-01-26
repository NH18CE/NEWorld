// 
// NWShared: Module.cpp
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

#include "Module.h"
#include <map>

namespace Module {
    namespace {
        class LoadHelper final {
        public:
            using LoadMap = std::map<std::string, LoadHelper>;
            LoadHelper(Module m, VersionInfo v, DependencyInfo d) noexcept : md(std::move(m)), mVersionInfo(v), mDependencies(d){}
            bool connect(LoadMap& map) {
                if (!mIsLoaded) {
                    try {
                        for (auto& x : mDependencies) 
                            if (auto it = map.find(x.first); it != map.end())
                                if (!(x.second <= it->second.mVersionInfo.first && x.second > it->second.mVersionInfo.second)) 
                                    if (!(mIsLoaded &= it->second.connect(map)))
                                        return false;
                        md.preInit();
                        mIsLoaded = true;
                    }
                    catch (...) {
                        mIsLoaded = false;
                        throw;
                    }
                }
                return mIsLoaded;
            }
            Module md;
        private:
            VersionInfo mVersionInfo;
            DependencyInfo mDependencies;
            bool mIsLoaded = false;
        };
    }

    Server::Server() {
        LoadHelper::LoadMap map;
        for (auto& x : filesystem::directory_iterator("Modules")) {
            Module md{ x };
            auto vi = md.getVersion();
            auto dep = md.getDependencies();
            map.insert({ x.path().filename().string(), std::move(LoadHelper(std::move(md), vi, dep)) });
        }
        for (auto& [name, module] : map) 
            if (module.connect(map))
                mModules.push_back(std::move(module.md));
        for (auto& x : mModules)
            x.init();
    }

    Server& Server::getInstance() noexcept {
        static Server srv{};
        return srv;
    }

    Server::~Server() = default;

    Module::Module(const filesystem::path& pth) : mLibrary((pth / "Entry.dll").string()) {}

    Module::~Module() {
        auto fn = mLibrary.get<void(*)()>("unInit");
        if (fn)
            fn();
    }

    void Module::init() {
        auto fn = mLibrary.get<void(*)()>("init");
        if (fn)
            fn();
    }

    void Module::preInit() {
        auto fn = mLibrary.get<void(*)()>("preInit");
        if (fn)
            fn();
    }

    const std::pair<Version, Version>& Module::getVersion() const {
        return mLibrary.get<VersionInfo(*)()>("getVersion")();
    }

    const std::vector<std::pair<std::string, Version>>& Module::getDependencies() const {
        return mLibrary.get<DependencyInfo(*)()>("getDependencies")();
    }
}
