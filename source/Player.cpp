// 
// NEWorld: Player.cpp
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

#include "Player.h"
#include "World.h"
#include <fstream>

int Player::gamemode = Survival;
bool Player::Glide;
bool Player::Flying;
bool Player::CrossWall;
double Player::glidingMinimumSpeed = pow(1, 2) / 2;

float Player::height = 1.2f;
float Player::heightExt = 0.0f;
bool Player::OnGround = false;
bool Player::Running = false;
bool Player::NearWall = false;
bool Player::inWater = false;
bool Player::glidingNow = false;
item Player::BlockInHand = Blocks::AIR;
uint8_t Player::indexInHand = 0;

Hitbox::AABB Player::playerbox;
std::vector<Hitbox::AABB> Player::Hitboxes;

double Player::xa, Player::ya, Player::za, Player::xd, Player::yd, Player::zd;
double Player::health = 20, Player::healthmax = 20, Player::healSpeed = 0.01, Player::dropDamage = 5.0;
onlineid Player::onlineID;
std::string Player::name;
Frustum Player::ViewFrustum;

double Player::speed;
int Player::AirJumps;
int Player::cxt, Player::cyt, Player::czt, Player::cxtl, Player::cytl, Player::cztl;
double Player::lookupdown, Player::heading, Player::xpos, Player::ypos, Player::zpos;
double Player::xposold, Player::yposold, Player::zposold, Player::jump;
double Player::xlookspeed, Player::ylookspeed;
int Player::intxpos, Player::intypos, Player::intzpos;
int Player::intxposold, Player::intyposold, Player::intzposold;

item Player::inventory[4][10];
short Player::inventoryAmount[4][10];

double Player::glidingEnergy, Player::glidingSpeed;

void InitHitbox(Hitbox::AABB& playerbox) {
    playerbox.xmin = -0.3;
    playerbox.xmax = 0.3;
    playerbox.ymin = -0.85;
    playerbox.ymax = 0.85;
    playerbox.zmin = -0.3;
    playerbox.zmax = 0.3;
}

void InitPosition() {
    Player::xposold = Player::xpos;
    Player::yposold = Player::ypos;
    Player::zposold = Player::zpos;
    Player::cxt = World::getChunkPos((int)Player::xpos);
    Player::cxtl = Player::cxt;
    Player::cyt = World::getChunkPos((int)Player::ypos);
    Player::cytl = Player::cyt;
    Player::czt = World::getChunkPos((int)Player::zpos);
    Player::cztl = Player::czt;
}

void MoveHitbox(double x, double y, double z) { moveTo(Player::playerbox, x, y + 0.5, z); }

void updateHitbox() { MoveHitbox(Player::xpos, Player::ypos, Player::zpos); }

void Player::init(double x, double y, double z) {
    xpos = x;
    ypos = y;
    zpos = z;
    InitHitbox(playerbox);
    InitPosition();
    updateHitbox();
}

void Player::spawn() {
    xpos = 0.0;
    ypos = 60.0;
    zpos = 0.0;
    jump = 0.0;
    InitHitbox(playerbox);
    InitPosition();
    updateHitbox();
    health = healthmax;
    memset(inventory, 0, sizeof(inventory));
    memset(inventoryAmount, 0, sizeof(inventoryAmount));

    //总得加点物品吧
    for (size_t i = 0; i < 255; i++) {
        addItem(Blocks::ROCK);
        addItem(Blocks::GRASS);
        addItem(Blocks::DIRT);
        addItem(Blocks::STONE);
        addItem(Blocks::PLANK);
        addItem(Blocks::WOOD);
        //addItem(Blocks::BEDROCK);TMD这个是基岩
        addItem(Blocks::LEAF);
        addItem(Blocks::GLASS);
        addItem(Blocks::WATER);
        addItem(Blocks::LAVA);
        addItem(Blocks::GLOWSTONE);
        addItem(Blocks::SAND);
        addItem(Blocks::CEMENT);
        addItem(Blocks::ICE);
        addItem(Blocks::COAL);
        addItem(Blocks::IRON);
        addItem(Blocks::TNT);
    }
}

void Player::updatePosition() {
    inWater = World::inWater(playerbox);
    if (!Flying && !CrossWall && inWater) {
        xa *= 0.6;
        ya *= 0.6;
        za *= 0.6;
    }
    double xal = xa, yal = ya, zal = za;

    if (!CrossWall) {
        Hitboxes.clear();
        Hitboxes = World::getHitboxes(expand(playerbox, xa, ya, za));
        int num = Hitboxes.size();
        if (num > 0) {
            for (int i = 0; i < num; i++) { ya = maxMoveOnYclip(playerbox, Hitboxes[i], ya); }
            move(playerbox, 0.0, ya, 0.0);
            for (int i = 0; i < num; i++) { xa = maxMoveOnXclip(playerbox, Hitboxes[i], xa); }
            move(playerbox, xa, 0.0, 0.0);
            for (int i = 0; i < num; i++) { za = maxMoveOnZclip(playerbox, Hitboxes[i], za); }
            move(playerbox, 0.0, 0.0, za);
        }
    }
    if (ya != yal && yal < 0.0) {
        OnGround = true;
        glidingEnergy = 0;
        glidingSpeed = 0;
        glidingNow = false;
        if (yal < -0.4 && gamemode == Survival) { health += yal * dropDamage; }
    }
    else OnGround = false;
    if (ya != yal && yal > 0.0) jump = 0.0;
    NearWall = xa != xal || za != zal;

    //消除浮点数精度带来的影响（查了好久的穿墙bug才发现是在这里有问题(╯‵□′)╯︵┻━┻）
    //  --qiaozhanrong
    xa = (double)((int)(xa * 100000)) / 100000.0;
    ya = (double)((int)(ya * 100000)) / 100000.0;
    za = (double)((int)(za * 100000)) / 100000.0;

    xd = xa;
    yd = ya;
    zd = za;
    xpos += xa;
    ypos += ya;
    zpos += za;
    xa *= 0.8;
    za *= 0.8;
    if (Flying || CrossWall) ya *= 0.8;
    if (OnGround) xa *= 0.7, ya = 0.0, za *= 0.7;
    updateHitbox();

    cxtl = cxt;
    cytl = cyt;
    cztl = czt;
    cxt = World::getChunkPos((int)xpos);
    cyt = World::getChunkPos((int)ypos);
    czt = World::getChunkPos((int)zpos);
}

bool Player::putBlock(int x, int y, int z, Block blockname) {
    Hitbox::AABB blockbox;
    bool success = false;
    blockbox.xmin = x - 0.5;
    blockbox.ymin = y - 0.5;
    blockbox.zmin = z - 0.5;
    blockbox.xmax = x + 0.5;
    blockbox.ymax = y + 0.5;
    blockbox.zmax = z + 0.5;
    if (((!hit(playerbox, blockbox) || CrossWall || !getBlockInfo(blockname)
                .
                isSolid()
            )
            &&
            !
            getBlockInfo(World::getBlock(x, y, z)).
            isSolid()
        )
    ) {
        World::setblock(x, y, z, blockname);
        success = true;
    }
    return success;
}

bool Player::save(const std::string& worldn) {
    uint32_t curversion = VERSION;
    std::stringstream ss;
    ss << "Worlds/" << worldn << "/player.NEWorldPlayer";
    std::ofstream isave(ss.str().c_str(), std::ios::binary | std::ios::out);
    if (!isave.is_open()) return false;
    isave.write((char*)&curversion, sizeof(curversion));
    isave.write((char*)&xpos, sizeof(xpos));
    isave.write((char*)&ypos, sizeof(ypos));
    isave.write((char*)&zpos, sizeof(zpos));
    isave.write((char*)&lookupdown, sizeof(lookupdown));
    isave.write((char*)&heading, sizeof(heading));
    isave.write((char*)&jump, sizeof(jump));
    isave.write((char*)&OnGround, sizeof(OnGround));
    isave.write((char*)&Running, sizeof(Running));
    isave.write((char*)&AirJumps, sizeof(AirJumps));
    isave.write((char*)&Flying, sizeof(Flying));
    isave.write((char*)&CrossWall, sizeof(CrossWall));
    isave.write((char*)&indexInHand, sizeof(indexInHand));
    isave.write((char*)&health, sizeof(health));
    isave.write((char*)&gamemode, sizeof(gamemode));
    isave.write((char*)&gametime, sizeof(gametime));
    isave.write((char*)inventory, sizeof(inventory));
    isave.write((char*)inventoryAmount, sizeof(inventoryAmount));
    isave.close();
    return true;
}

bool Player::load(const std::string& worldn) {
    uint32_t targetVersion;
    std::stringstream ss;
    ss << "Worlds/" << worldn << "/player.NEWorldPlayer";
    std::ifstream iload(ss.str().c_str(), std::ios::binary | std::ios::in);
    if (!iload.is_open()) return false;
    iload.read((char*)&targetVersion, sizeof(targetVersion));
    if (targetVersion != VERSION) return false;
    iload.read((char*)&xpos, sizeof(xpos));
    iload.read((char*)&ypos, sizeof(ypos));
    iload.read((char*)&zpos, sizeof(zpos));
    iload.read((char*)&lookupdown, sizeof(lookupdown));
    iload.read((char*)&heading, sizeof(heading));
    iload.read((char*)&jump, sizeof(jump));
    iload.read((char*)&OnGround, sizeof(OnGround));
    iload.read((char*)&Running, sizeof(Running));
    iload.read((char*)&AirJumps, sizeof(AirJumps));
    iload.read((char*)&Flying, sizeof(Flying));
    iload.read((char*)&CrossWall, sizeof(CrossWall));
    iload.read((char*)&indexInHand, sizeof(indexInHand));
    iload.read((char*)&health, sizeof(health));
    iload.read((char*)&gamemode, sizeof(gamemode));
    iload.read((char*)&gametime, sizeof(gametime));
    iload.read((char*)inventory, sizeof(inventory));
    iload.read((char*)inventoryAmount, sizeof(inventoryAmount));
    iload.close();
    return true;
}

bool Player::addItem(item itemname, short amount) {
    //向背包里加入物品
    const int InvmaxStack = 255;
    for (int i = 3; i >= 0; i--) {
        for (int j = 0; j != 10; j++) {
            if (inventory[i][j] == itemname && inventoryAmount[i][j] < InvmaxStack) {
                //找到一个同类格子
                if (amount + inventoryAmount[i][j] <= InvmaxStack) {
                    inventoryAmount[i][j] += amount;
                    return true;
                }
                amount -= InvmaxStack - inventoryAmount[i][j];
                inventoryAmount[i][j] = InvmaxStack;
            }
        }
    }
    for (int i = 3; i >= 0; i--) {
        for (int j = 0; j != 10; j++) {
            if (inventory[i][j] == Blocks::AIR) {
                //找到一个空白格子
                inventory[i][j] = itemname;
                if (amount <= InvmaxStack) {
                    inventoryAmount[i][j] = amount;
                    return true;
                }
                inventoryAmount[i][j] = InvmaxStack;
                amount -= InvmaxStack;
            }
        }
    }
    return false;
}

void Player::changeGameMode(int _gamemode) {
    gamemode = _gamemode;
    switch (_gamemode) {
    case Survival:
        Flying = false;
        jump = 0.0;
        CrossWall = false;
        break;

    case Creative:
        Flying = true;
        jump = 0.0;
        break;
    }
}
