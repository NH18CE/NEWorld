// 
// NEWorld: API.cpp
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

#include "API.h"
#include "Player.h"
#include "World.h"

std::map<std::string, void*> Module::sharedData;

APIPackage Module::getPackage() {
    static APIPackage api;
    static bool init = false;
    if (init) return api;
    api.getChunk = World::getChunkPtr;
    api.getBlock = [](int x, int y, int z) { return World::getBlock(x, y, z); };
    api.setBlock = [](int x, int y, int z, Block blk) { World::setblock(x, y, z, blk); };
    api.getCommand = [](const std::string& s) -> Command*
                            {
        for (auto& command : commands)
            if (command.identifier == s) return &command;
        return nullptr;
    };
    api.registerCommand = [](Command c) -> bool {
        for (auto& command : commands)
            if (command.identifier == c.identifier) return false;
        commands.push_back(c);
        return true;
    };
    api.getSharedData = [](const std::string& key) -> void*
                            {
        std::map<std::string, void*>::iterator iter = sharedData.find(key);
        if (iter == sharedData.end()) return nullptr;
        return iter->second;
    };
    api.setSharedData = [](const std::string& key, void* value) { sharedData[key] = value; };
    api.getPlayerData = []()-> PlayerData {
        PlayerData player;
        player.AirJumps = Player::AirJumps;
        player.BlockInHand = Player::BlockInHand;
        player.CrossWall = Player::CrossWall;
        player.cxt = Player::cxt;
        player.cxtl = Player::cxtl;
        player.cyt = Player::cyt;
        player.cytl = Player::cytl;
        player.czt = Player::czt;
        player.cztl = Player::cztl;
        player.dropDamage = Player::dropDamage;
        player.Flying = Player::Flying;
        player.Glide = Player::Glide;
        player.glidingEnergy = Player::glidingEnergy;
        player.glidingMinimumSpeed = Player::glidingMinimumSpeed;
        player.glidingNow = Player::glidingNow;
        player.glidingSpeed = Player::glidingSpeed;
        player.heading = Player::heading;
        player.healSpeed = Player::healSpeed;
        player.health = Player::health;
        player.healthmax = Player::healthmax;
        player.height = Player::height;
        player.heightExt = Player::heightExt;
        //player.Hitboxes = Player::Hitboxes;
        player.indexInHand = Player::indexInHand;
        player.inventory = (item*)Player::inventory;
        player.inventoryAmount = (short*)Player::inventoryAmount;
        player.inWater = Player::inWater;
        player.jump = Player::jump;
        player.lookupdown = Player::lookupdown;
        //player.name = Player::name;
        player.NearWall = Player::NearWall;
        player.OnGround = Player::OnGround;
        player.onlineID = Player::onlineID;
        player.playerbox = Player::playerbox;
        player.Running = Player::Running;
        player.speed = Player::speed;
        player.ViewFrustum = Player::ViewFrustum;
        player.xa = Player::xa;
        player.xd = Player::xd;
        player.xlookspeed = Player::xlookspeed;
        player.xpos = Player::xpos;
        player.xposold = Player::xposold;
        player.ya = Player::ya;
        player.yd = Player::yd;
        player.ylookspeed = Player::ylookspeed;
        player.ypos = Player::ypos;
        player.yposold = Player::yposold;
        player.za = Player::za;
        player.zd = Player::zd;
        player.zpos = Player::zpos;
        player.zposold = Player::zposold;
        return player;
    };
    api.updateBlock = [](int x, int y, int z, bool chgd) { World::updateblock(x, y, z, chgd); };
    api.setChunkUpdated = World::setChunkUpdated;
    init = true;
    return api;
}
