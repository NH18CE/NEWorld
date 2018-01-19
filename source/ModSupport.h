// 
// NEWorld: ModSupport.h
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
#include <string>
#include <vector>
#include "Hitbox.h"
#include "Frustum.h"
#include "Command.h"
#include "Chunk.h"
#include "Utils/Dylib.h"

typedef unsigned char uint8_t;
typedef unsigned int onlineid;
typedef unsigned short item;
typedef unsigned short Block;

struct ModInfo {
    char name[128];
    char version[128];
    char dependence[128];
    Library lib;
};

struct PlayerData {
    Hitbox::AABB playerbox;
    //std::vector<Hitbox::AABB> Hitboxes;
    double xa, ya, za, xd, yd, zd;
    double health, healthmax, healSpeed, dropDamage;
    onlineid onlineID;
    //std::string name;
    Frustum ViewFrustum;
    bool Glide;
    bool Flying;
    bool CrossWall;
    double glidingMinimumSpeed;
    bool OnGround;
    bool Running;
    bool NearWall;
    bool inWater;
    bool glidingNow;
    double speed;
    int AirJumps;
    int cxt, cyt, czt, cxtl, cytl, cztl;
    double lookupdown, heading, xpos, ypos, zpos, xposold, yposold, zposold, jump;
    double xlookspeed, ylookspeed;
    float height;
    float heightExt;
    item BlockInHand;
    uint8_t indexInHand;
    item* inventory;
    short* inventoryAmount;
    double glidingEnergy, glidingSpeed;
};

struct APIPackage {
    std::function<World::Chunk*(int cx, int cy, int cz)> getChunk;
    std::function<Block(int cx, int cy, int cz)> getBlock;
    std::function<void(int x, int y, int z, Block Block)> setBlock;
    std::function<Command*(const std::string& commandName)> getCommand;
    std::function<bool(Command command)> registerCommand;
    std::function<void*(const std::string& key)> getSharedData;
    std::function<void(const std::string& key, void* value)> setSharedData;
    std::function<PlayerData()> getPlayerData;
    std::function<void(int x, int y, int z, bool blockchanged)> updateBlock;
    std::function<void(int x, int y, int z, bool value)> setChunkUpdated;
};
