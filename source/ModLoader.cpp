// 
// NEWorld: ModLoader.cpp
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

#include "ModLoader.h"
#include <functional>
#include <iostream>
#include <fstream>
#include "API.h"

std::vector<ModInfo> Module::ModLoader::mods;

//查找一个文件夹下所有的子目录
std::vector<std::string> findFolders(const std::string& path) {
    std::vector<std::string> ret;
    for (auto&& x : filesystem::directory_iterator(path))
        if (is_directory(x.path()))
            ret.push_back(x.path().string());
    return ret;
}

namespace Module {
    struct ModuleInfo {
        std::string name;
        Version version;
        Version downwardCompatibility;
        Library lib;
    };

    // Loaded, Dependencies, ModuleInfo
    struct LoaderEnt {
        bool isLoaded;
        std::vector<DependencyInfo> dependencies;
        ModuleInfo thisInfo;
    };

    std::map<std::string, LoaderEnt> loaderGraph;

    // Info List Lexical Format
    // Version ::= 'v'int'.'int'.'int'.'int
    // Dependency ::= string Version
    // Dependencies ::= eps | Dependency | Dependency Dependencies
    // File ::= Version Version Dependencies

    std::istream& operator >>(std::istream& stream, Version& ver) {
        stream.ignore(0xEFFFFFFF, 'v');
        for (auto i = 0; i < 4; ++i) {
            auto temp = 0;
            while (isdigit(stream.peek())) (temp *= 10) += (stream.get() - '0');
            switch (i) {
            case 0: ver.vMajor = temp;
                stream.get();
                break;
            case 1: ver.vMinor = temp;
                stream.get();
                break;
            case 2: ver.vRevision = temp;
                stream.get();
                break;
            case 3: ver.vBuild = temp;
                break;
            default: break;
            };
        }
        return stream;
    }

    void loadModuleInfo(const std::string& name) {
        LoaderEnt ent;
        ent.thisInfo.name = name;
        std::ifstream info{"Mods/" + name + "MetaInfo.txt"};
        if (info.good()) {
            info >> ent.thisInfo.version >> ent.thisInfo.version;
            for (;;) {
                DependencyInfo dinfo;
                info >> dinfo.name >> dinfo.version;
                if (!info.fail())
                    ent.dependencies.push_back(std::move(dinfo));
                else
                    break;
            }
            loaderGraph.emplace(name, std::move(ent));
        }
    }

    void load(LoaderEnt& meta) {
        if (!meta.dependencies.empty())
            for (auto& x : meta.dependencies)
                load(loaderGraph[x.name]);
        meta.thisInfo.lib.load("Mods/" + meta.thisInfo.name + "/Entry.dll");
        meta.thisInfo.lib.get<void(*)(APIPackage)>("coInitialize")(getPackage());
        meta.isLoaded = true;
    }

    void walk() {
        for (auto& x : findFolders("Mods/")) //获得Mods/下所有的文件夹 
            loadModuleInfo(x);
    }

    void load() {
        for (auto& [name, meta] : loaderGraph)
            if (!meta.isLoaded)
                load(meta);
    }
}

void Module::ModLoader::loadMods() {
    std::vector<std::string> modLoadList = findFolders("Mods/"); //获得Mods/下所有的文件夹
    bool successAtLeastOne = false;
    do {
        //循环加载Module，直到某一次循环一个成功加载的都没有为止（因为依赖项）
        successAtLeastOne = false;
        for (std::vector<std::string>::iterator iter = modLoadList.begin(); iter != modLoadList.end();) {
            ModLoadStatus status = loadSingleMod("Mods/" + *iter + "/entry.dll");
            if (status == Success) successAtLeastOne = true;
            if (status != MissDependence) iter = modLoadList.erase(iter); //只要不是因缺少依赖而失败就删掉记录
            else ++iter;
        }
    }
    while (successAtLeastOne);
    std::cout << "[Console][Game]Load complete! "
        << mods.size() << " mod" << (mods.size() > 1 ? "s" : "") << " loaded." << std::endl;
}

//加载单个Module，loadMods会调用该函数
Module::ModLoader::ModLoadStatus Module::ModLoader::loadSingleMod(const std::string& modPath) {
    Library lib{modPath};
    auto init = lib.get<bool(*)(APIPackage)>("init");
    auto info = lib.get<ModInfo(*)()>("getModInfo")(); //获得Module信息
    std::cout << "[Console][Game]Loading Mod " << info.name << "[" << info.version << "]" << std::endl;
    if (strcmp(info.dependence, "") != 0) {
        //判断并检查依赖项
        bool foundDependence = false;
        for (auto& mod : mods) {
            if (mod.name == info.dependence) {
                foundDependence = true;
                break;
            }
        }
        if (!foundDependence) return MissDependence;
    }
    if (!(*init)(getPackage())) return InitFailed; //初始化Module
    info.lib = std::move(lib);
    mods.push_back(std::move(info));
    return Success;
}

void Module::ModLoader::unloadMods() { mods.clear(); }
