// 
// NEWorld: GameView.cpp
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

#include "GameView.h"
#include "Textures.h"
#include "TextRenderer.h"
#include "ShadowMaps.h"
#include "Menus.h"
#include "Command.h"
#include "Setup.h"

ThreadFunc updateThreadFunc(void*);

int getMouseScroll() { return mw; }
int getMouseButton() { return mb; }
std::vector<Command> commands;

class GameDView : public GUI::Form {
private:
    std::string chatword;
    bool chatmode = false;
    std::vector<std::string> chatMessages;

    int fps, fpsc, ups, upsc;
    double fctime, uctime;

    bool GUIrenderswitch;
    bool DebugMode;
    bool DebugHitbox;
    bool DebugShadow;
    bool DebugMergeFace;

    int selx, sely, selz, oldselx, oldsely, oldselz, selface;
    bool sel;
    float selt, seldes;
    Block selb;
    Brightness selbr;
    bool selce;
    int selbx, selby, selbz, selcx, selcy, selcz;

    int getMouseScroll() { return mw; }
    int getMouseButton() { return mb; }
public:
    void GameThreadloop() {
        //Wait until start...
        MutexLock(Mutex);
        while (!updateThreadRun) {
            MutexUnlock(Mutex);
            std::this_thread::yield();
            MutexLock(Mutex);
        }
        MutexUnlock(Mutex);

        //Thread start
        MutexLock(Mutex);
        lastupdate = timer();

        while (updateThreadRun) {
            MutexUnlock(Mutex);
            std::this_thread::yield(); //Don't make it always busy
            MutexLock(Mutex);

            while (updateThreadPaused) {
                MutexUnlock(Mutex);
                std::this_thread::yield(); //Same as before
                MutexLock(Mutex);
                lastupdate = updateTimer = timer();
            }

            FirstUpdateThisFrame = true;
            updateTimer = timer();
            if (updateTimer - lastupdate >= 5.0) lastupdate = updateTimer;

            while (updateTimer - lastupdate >= 1.0 / 30.0 && upsc < 60) {
                lastupdate += 1.0 / 30.0;
                upsc++;
                updategame();
                FirstUpdateThisFrame = false;
            }

            if (timer() - uctime >= 1.0) {
                uctime = timer();
                ups = upsc;
                upsc = 0;
            }
        }
        MutexUnlock(Mutex);
    }

    void saveGame() { Player::save(World::worldname); }

    bool loadGame() { return Player::load(World::worldname); }


    bool isPressed(int key, bool setFalse = false) {
        static bool keyPressed[GLFW_KEY_LAST + 1];
        if (setFalse) {
            keyPressed[key] = false;
            return true;
        }
        if (key > GLFW_KEY_LAST || key <= 0) return false;
        if (!glfwGetKey(MainWindow, key)) keyPressed[key] = false;
        if (!keyPressed[key] && glfwGetKey(MainWindow, key)) {
            keyPressed[key] = true;
            return true;
        }
        return false;
    }
	//方块交互相关
	std::pair<std::array<ALfloat,3>,bool> blockInteraction()
	{
		double lx = Player::xpos;
		double ly = Player::ypos + Player::height + Player::heightExt;
		double lz = Player::zpos;
		std::array<ALfloat, 3>BlockPos;
		bool BlockClick;
		//从玩家位置发射一条线段
		for (int i = 0; i < selectPrecision * selectDistance; i++) {
			double lxl = lx;
			double lyl = ly;
			double lzl = lz;

			//线段延伸
			lx += sin(M_PI / 180 * (Player::heading - 180)) * sin(M_PI / 180 * (Player::lookupdown + 90)) / (double)
				selectPrecision;
			ly += cos(M_PI / 180 * (Player::lookupdown + 90)) / (double)selectPrecision;
			lz += cos(M_PI / 180 * (Player::heading - 180)) * sin(M_PI / 180 * (Player::lookupdown + 90)) / (double)
				selectPrecision;

			//碰到方块
			if (getBlockInfo(World::getBlock(lround(lx), lround(ly), lround(lz))).isSolid()) {
				int x = lround(lx);
				int y = lround(ly);
				int z = lround(lz);
				int xl = lround(lxl);
				int yl = lround(lyl);
				int zl = lround(lzl);

				selx = x;
				sely = y;
				selz = z;
				sel = true;

				//找方块所在区块及位置
				selcx = World::getChunkPos(x);
				selcy = World::getChunkPos(y);
				selcz = World::getChunkPos(z);
				selbx = World::getBlockPos(x);
				selby = World::getBlockPos(y);
				selbz = World::getBlockPos(z);

				if (auto cp = World::getChunkPtr(selcx, selcy, selcz); cp && cp != World::emptyChunkPtr)
					selb = cp->getBlock(selbx, selby, selbz);

				selbr = World::getBrightness(xl, yl, zl);
				selb = World::getBlock(x, y, z);
				if (mb == 1 || glfwGetKey(MainWindow, GLFW_KEY_ENTER) == GLFW_PRESS) {
					Particles::throwParticle(selb,
						float(x + rnd() - 0.5f), float(y + rnd() - 0.2f),
						float(z + rnd() - 0.5f),
						float(rnd() * 0.2f - 0.1f), float(rnd() * 0.2f - 0.1f),
						float(rnd() * 0.2f - 0.1f),
						float(rnd() * 0.01f + 0.02f), int(rnd() * 30) + 30);

					if (selx != oldselx || sely != oldsely || selz != oldselz) seldes = 0.0;
					else {
						float Factor = 1.0;
						if (Player::inventory[3][Player::indexInHand] == STICK)Factor = 4;
						else
							Factor = 30.0 / (getBlockInfo(Player::inventory[3][Player::indexInHand]).getHardness() +
								0.1);
						if (Factor < 1.0)Factor = 1.0;
						if (Factor > 1.7)Factor = 1.7;
						seldes += getBlockInfo(selb).getHardness() * (Player::gamemode == Player::Creative
							? 10.0f
							: 0.3f) * Factor;
						BlockClick = true;
						BlockPos[0] = x;
						BlockPos[1] = y;
						BlockPos[2] = z;
					}

					if (seldes >= 100.0) {
						for (int j = 1; j <= 25; j++) {
							Particles::throwParticle(selb,
								float(x + rnd() - 0.5f), float(y + rnd() - 0.2f),
								float(z + rnd() - 0.5f),
								float(rnd() * 0.2f - 0.1f), float(rnd() * 0.2f - 0.1f),
								float(rnd() * 0.2f - 0.1f),
								float(rnd() * 0.02 + 0.03), int(rnd() * 60) + 30);
						}
						World::pickblock(x, y, z);
						BlockClick = true;
						BlockPos[0] = x;
						BlockPos[1] = y;
						BlockPos[2] = z;
					}
				}
				if ((mb == 2 && !static_cast<bool>(mbp)) || (!chatmode && isPressed(GLFW_KEY_TAB))) {
					//鼠标右键
					if (Player::inventoryAmount[3][Player::indexInHand] > 0 && isBlock(
						Player::inventory[3][Player::indexInHand])) {
						//放置方块
						if (Player::putBlock(xl, yl, zl, Player::BlockInHand)) {
							Player::inventoryAmount[3][Player::indexInHand]--;
							if (Player::inventoryAmount[3][Player::indexInHand] == 0)
								Player::inventory[3][Player::
								indexInHand] = Blocks::AIR;

							BlockClick = true;
							BlockPos[0] = x;
							BlockPos[1] = y;
							BlockPos[2] = z;
						}
					}
					else {
						//使用物品
						if (Player::inventory[3][Player::indexInHand] == APPLE) {
							Player::inventoryAmount[3][Player::indexInHand]--;
							if (Player::inventoryAmount[3][Player::indexInHand] == 0)
								Player::inventory[3][Player::
								indexInHand] = Blocks::AIR;
							Player::health = Player::healthmax;
						}
					}
				}
				break;
			}
		}
		return std::make_pair(BlockPos,BlockClick);
	}
	//物理动作相关
	void playerAction(bool& WP,double& Wprstm)
	{
		//移动！(生命在于运动)
		if (glfwGetKey(MainWindow, GLFW_KEY_W) || Player::glidingNow) {
			if (!WP) {
				if (Wprstm == 0.0) { Wprstm = timer(); }
				else {
					if (timer() - Wprstm <= 0.5) {
						Player::Running = true;
						Wprstm = 0.0;
					}
					else Wprstm = timer();
				}
			}
			if (Wprstm != 0.0 && timer() - Wprstm > 0.5) Wprstm = 0.0;
			WP = true;
			if (!Player::glidingNow) {
				Player::xa += -sin(Player::heading * M_PI / 180.0) * Player::speed;
				Player::za += -cos(Player::heading * M_PI / 180.0) * Player::speed;
			}
			else {
				Player::xa = sin(M_PI / 180 * (Player::heading - 180)) * sin(
					M_PI / 180 * (Player::lookupdown + 90)) * Player::glidingSpeed * speedCast;
				Player::ya = cos(M_PI / 180 * (Player::lookupdown + 90)) * Player::glidingSpeed * speedCast;
				Player::za = cos(M_PI / 180 * (Player::heading - 180)) * sin(
					M_PI / 180 * (Player::lookupdown + 90)) * Player::glidingSpeed * speedCast;
				if (Player::ya < 0) Player::ya *= 2;
			}
		}
		else {
			Player::Running = false;
			WP = false;
		}
		if (Player::Running)Player::speed = runspeed;
		else Player::speed = walkspeed;

		if (glfwGetKey(MainWindow, GLFW_KEY_S) == GLFW_PRESS && !Player::glidingNow) {
			Player::xa += sin(Player::heading * M_PI / 180.0) * Player::speed;
			Player::za += cos(Player::heading * M_PI / 180.0) * Player::speed;
			Wprstm = 0.0;
		}

		if (glfwGetKey(MainWindow, GLFW_KEY_A) == GLFW_PRESS && !Player::glidingNow) {
			Player::xa += sin((Player::heading - 90) * M_PI / 180.0) * Player::speed;
			Player::za += cos((Player::heading - 90) * M_PI / 180.0) * Player::speed;
			Wprstm = 0.0;
		}

		if (glfwGetKey(MainWindow, GLFW_KEY_D) == GLFW_PRESS && !Player::glidingNow) {
			Player::xa += -sin((Player::heading - 90) * M_PI / 180.0) * Player::speed;
			Player::za += -cos((Player::heading - 90) * M_PI / 180.0) * Player::speed;
			Wprstm = 0.0;
		}

		if (!Player::Flying && !Player::CrossWall) {
			double horizontalSpeed = sqrt(Player::xa * Player::xa + Player::za * Player::za);
			if (horizontalSpeed > Player::speed && !Player::glidingNow) {
				Player::xa *= Player::speed / horizontalSpeed;
				Player::za *= Player::speed / horizontalSpeed;
			}
		}
		else {
			if (glfwGetKey(MainWindow, GLFW_KEY_R) == GLFW_PRESS && !Player::glidingNow) {
				if (glfwGetKey(MainWindow, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
					Player::xa = -sin(Player::heading * M_PI / 180.0) * runspeed * 10;
					Player::za = -cos(Player::heading * M_PI / 180.0) * runspeed * 10;
				}
				else {
					Player::xa = sin(M_PI / 180 * (Player::heading - 180)) * sin(
						M_PI / 180 * (Player::lookupdown + 90)) * runspeed * 20;
					Player::ya = cos(M_PI / 180 * (Player::lookupdown + 90)) * runspeed * 20;
					Player::za = cos(M_PI / 180 * (Player::heading - 180)) * sin(
						M_PI / 180 * (Player::lookupdown + 90)) * runspeed * 20;
				}
			}

			if (glfwGetKey(MainWindow, GLFW_KEY_F) == GLFW_PRESS && !Player::glidingNow) {
				if (glfwGetKey(MainWindow, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
					Player::xa = sin(Player::heading * M_PI / 180.0) * runspeed * 10;
					Player::za = cos(Player::heading * M_PI / 180.0) * runspeed * 10;
				}
				else {
					Player::xa = -sin(M_PI / 180 * (Player::heading - 180)) * sin(
						M_PI / 180 * (Player::lookupdown + 90)) * runspeed * 20;
					Player::ya = -cos(M_PI / 180 * (Player::lookupdown + 90)) * runspeed * 20;
					Player::za = -cos(M_PI / 180 * (Player::heading - 180)) * sin(
						M_PI / 180 * (Player::lookupdown + 90)) * runspeed * 20;
				}
			}
		}

		//切换方块
		if (isPressed(GLFW_KEY_Z) && Player::indexInHand > 0) Player::indexInHand--;
		if (isPressed(GLFW_KEY_X) && Player::indexInHand < 9) Player::indexInHand++;
		if ((int)Player::indexInHand + (mwl - mw) < 0)Player::indexInHand = 0;
		else if ((int)Player::indexInHand + (mwl - mw) > 9)Player::indexInHand = 9;
		else Player::indexInHand += (char)(mwl - mw);
		mwl = mw;

		//起跳！
		if (isPressed(GLFW_KEY_SPACE)) {
			if (!Player::inWater) {
				if ((Player::OnGround || Player::AirJumps < maxAirJumps) && !Player::Flying && !Player::
					CrossWall) {
					if (!Player::OnGround) {
						Player::jump = 0.3;
						Player::AirJumps++;
					}
					else {
						Player::jump = 0.25;
						Player::OnGround = false;
					}
				}
				if (Player::Flying || Player::CrossWall) {
					Player::ya += walkspeed / 2;
					isPressed(GLFW_KEY_SPACE, true);
				}
				Wprstm = 0.0;
			}
			else {
				Player::ya = walkspeed;
				isPressed(GLFW_KEY_SPACE, true);
			}
		}

		if ((glfwGetKey(MainWindow, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS || glfwGetKey(
			MainWindow, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS) && !Player::glidingNow) {
			if (Player::CrossWall || Player::Flying) Player::ya -= walkspeed / 2;
			Wprstm = 0.0;
		}

		if (glfwGetKey(MainWindow, GLFW_KEY_K) && Player::Glide && !Player::OnGround && !Player::glidingNow) {
			double h = Player::ypos + Player::height + Player::heightExt;
			Player::glidingEnergy = g * h;
			Player::glidingSpeed = 0;
			Player::glidingNow = true;
		}

		//各种设置切换
		if (isPressed(GLFW_KEY_F1)) {
			Player::changeGameMode(Player::gamemode == Player::Creative ? Player::Survival : Player::Creative);
		}
		if (isPressed(GLFW_KEY_F2)) shouldGetScreenshot = true;
		if (isPressed(GLFW_KEY_F3)) DebugMode = !DebugMode;
		if (isPressed(GLFW_KEY_F4)) Player::CrossWall = !Player::CrossWall;
		if (isPressed(GLFW_KEY_H) && glfwGetKey(MainWindow, GLFW_KEY_F3) == GLFW_PRESS) {
			DebugHitbox = !DebugHitbox;
			DebugMode = true;
		}
		if (Renderer::AdvancedRender) {
			if (isPressed(GLFW_KEY_M) && glfwGetKey(MainWindow, GLFW_KEY_F3) == GLFW_PRESS) {
				DebugShadow = !DebugShadow;
				DebugMode = true;
			}
		}
		else DebugShadow = false;
		if (isPressed(GLFW_KEY_G) && glfwGetKey(MainWindow, GLFW_KEY_F3) == GLFW_PRESS) {
			DebugMergeFace = !DebugMergeFace;
			DebugMode = true;
		}
		if (isPressed(GLFW_KEY_F4) && Player::gamemode == Player::Creative)
			Player::CrossWall = !Player::CrossWall;
		if (isPressed(GLFW_KEY_F5)) GUIrenderswitch = !GUIrenderswitch;
		if (isPressed(GLFW_KEY_F6)) Player::Glide = !Player::Glide;
		if (isPressed(GLFW_KEY_F7)) Player::spawn();
		if (isPressed(GLFW_KEY_SLASH)) chatmode = true; //斜杠将会在下面的if(chatmode)里添加
	}
	void playerJump()
	{
		//跳跃
		if (!Player::glidingNow) {
			if (!Player::inWater) {
				if (!Player::Flying && !Player::CrossWall) {
					Player::ya = -0.001;
					if (Player::OnGround) {
						Player::jump = 0.0;
						Player::AirJumps = 0;
						isPressed(GLFW_KEY_SPACE, true);
					}
					else {
						//自由落体计算
						Player::jump -= 0.025;
						Player::ya = Player::jump + 0.5 * 0.6 / 900.0;
					}
				}
				else {
					Player::jump = 0.0;
					Player::AirJumps = 0;
				}
			}
			else {
				Player::jump = 0.0;
				Player::AirJumps = maxAirJumps;
				isPressed(GLFW_KEY_SPACE, true);
				if (Player::ya <= 0.001 && !Player::Flying && !Player::CrossWall) {
					Player::ya = -0.001;
					if (!Player::OnGround) Player::ya -= 0.1;
				}
			}
		}
	}
	//滑翔相关
	void playerGlid()
	{
		if (Player::glidingNow) {
			double& E = Player::glidingEnergy;
			double oldh = Player::ypos + Player::height + Player::heightExt + Player::ya;
			double h = oldh;
			if (E - Player::glidingMinimumSpeed < h * g) {
				//小于最小速度
				h = (E - Player::glidingMinimumSpeed) / g;
			}
			Player::glidingSpeed = sqrt(2 * (E - g * h));
			E -= EDrop;
			Player::ya += h - oldh;
		}
	}
    void updategame() {
        //Time_updategame_ = timer();
		static double Wprstm;
		static bool WP;
        //bool chunkupdated = false;


        Player::BlockInHand = Player::inventory[3][Player::indexInHand];
        //生命值相关
        if (Player::health > 0 || Player::gamemode == Player::Creative) {
            if (Player::ypos < -100) Player::health -= (-100 - Player::ypos) / 100;
            if (Player::health < Player::healthmax) Player::health += Player::healSpeed;
            if (Player::health > Player::healthmax) Player::health = Player::healthmax;
        }
        else Player::spawn();

        //时间
        gametime++;
        if (glfwGetKey(MainWindow, GLFW_KEY_F8)) gametime += 30;
        if (gametime > gameTimemax) gametime = 0;

        //World::unloadedChunks=0
        World::rebuiltChunks = 0;
        World::updatedChunks = 0;

        //cpArray move
        World::cpArray.moveTo(Player::cxt - viewdistance - 2, Player::cyt - viewdistance - 2,
                              Player::czt - viewdistance - 2);

        //HeightMap move
        World::hMap.moveTo((Player::cxt - viewdistance - 2) * 16, (Player::czt - viewdistance - 2) * 16);

        if (FirstUpdateThisFrame)
            World::updateChunkLoading();

        //加载动画
        for (int i = 0; i < World::loadedChunks; i++) {
            World::Chunk* cp = World::chunks[i];
            if (cp->loadAnim <= 0.3f) cp->loadAnim = 0.0f;
            else cp->loadAnim *= 0.6f;
        }

        World::randomChunkUpdation();

        

        sel = false;
        selx = sely = selz = selbx = selby = selbz = selcx = selcy = selcz = selb = selbr = 0;
		//用于音效更新
		bool BlockClick = false;
		ALfloat BlockPos[3];
        if (!bagOpened) {
			
			auto blockInfo=blockInteraction();
			BlockPos[0] = blockInfo.first[0];
			BlockPos[1] = blockInfo.first[1];
			BlockPos[2] = blockInfo.first[2];
			BlockClick = blockInfo.second;
            if (selx != oldselx || sely != oldsely || selz != oldselz || mb == 0 && glfwGetKey(
                MainWindow, GLFW_KEY_ENTER) != GLFW_PRESS)
                seldes = 0.0;
            oldselx = selx;
            oldsely = sely;
            oldselz = selz;

            Player::intxpos = lround(Player::xpos);
            Player::intypos = lround(Player::ypos);
            Player::intzpos = lround(Player::zpos);

            //更新方向
            Player::heading += Player::xlookspeed;
            Player::lookupdown += Player::ylookspeed;
            Player::xlookspeed = Player::ylookspeed = 0.0;

            if (!chatmode) {
				playerAction(WP,Wprstm);
            }

            if (isPressed(GLFW_KEY_ENTER) == GLFW_PRESS) {
                chatmode = !chatmode;
                if (!chatword.empty()) {
                    //指令的执行，或发出聊天文本
                    if (chatword.substr(0, 1) == "/") {
                        //指令
                        std::vector<std::string> command = split(chatword, " ");
                        if (!doCommand(command)) {
                            //执行失败
                            DebugWarning("Fail to execute the command: " + chatword);
                            chatMessages.push_back("Fail to execute the command: " + chatword);
                        }
                    }
                    else { chatMessages.push_back(chatword); }
                }
                chatword = "";
            }
            if (chatmode) {
                if (isPressed(GLFW_KEY_BACKSPACE) && chatword.length() > 0) {
                    int n = chatword[chatword.length() - 1];
                    if (n > 0 && n <= 127)
                        chatword = chatword.substr(0, chatword.length() - 1);
                    else
                        chatword = chatword.substr(0, chatword.length() - 2);
                }
                else { chatword += inputstr; }
                //自动补全
                if (isPressed(GLFW_KEY_TAB) && chatmode && !chatword.empty() && chatword.substr(0, 1) == "/") {
                    for (unsigned int i = 0; i != commands.size(); i++) {
                        if (beginWith(commands[i].identifier, chatword)) { chatword = commands[i].identifier; }
                    }
                }
            }
        }

        inputstr = "";

        if (isPressed(GLFW_KEY_E) && GUIrenderswitch && !chatmode) {
            bagOpened = !bagOpened;
            bagAnimTimer = timer();
            if (!bagOpened) { glfwSetInputMode(MainWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED); }
            else {
                shouldGetThumbnail = true;
                Player::xlookspeed = Player::ylookspeed = 0.0;
            }
        }
		playerJump();
        //爬墙
        //if (Player::NearWall && Player::Flying == false && Player::CrossWall == false){
        //    Player::ya += walkspeed
        //    Player::jump = 0.0
        //}
		playerGlid();
        


        //音效更新
        int Run = 0;
        if (WP)Run = Player::Running ? 2 : 1;
        ALfloat PlayerPos[3];
        PlayerPos[0] = Player::xpos;
        PlayerPos[1] = Player::ypos;
        PlayerPos[2] = Player::zpos;
        bool Fall = Player::OnGround
            && !Player::inWater
            && Player::jump == 0;
        //更新声速
        AudioSystem::SpeedOfSound = Player::inWater ? AudioSystem::Water_SpeedOfSound : AudioSystem::Air_SpeedOfSound;
        //更新环境
        if (Player::inWater) { EFX::EAXprop = UnderWater; }
        else {
            if (Player::OnGround) { EFX::EAXprop = Plain; }
            else { EFX::EAXprop = Generic; }
        }
        EFX::UpdateEAXprop();
        AudioSystem::Update(PlayerPos, Fall, BlockClick, BlockPos, Run, Player::inWater);


        mbp = mb;
        FirstFrameThisUpdate = true;
        Particles::updateall();

        Player::intxpos = lround(Player::xpos);
        Player::intypos = lround(Player::ypos);
        Player::intzpos = lround(Player::zpos);
        Player::updatePosition();
        Player::xposold = Player::xpos;
        Player::yposold = Player::ypos;
        Player::zposold = Player::zpos;
        Player::intxposold = lround(Player::xpos);
        Player::intyposold = lround(Player::ypos);
        Player::intzposold = lround(Player::zpos);
    }

    void debugText(const std::string& s, bool init) {
        static int pos = 0;
        if (init) {
            pos = 0;
            return;
        }
        TextRenderer::renderAsciiString(0, 16 * pos, s);
        pos++;
    }

    void Grender() {
        //画场景
        double curtime = timer();
        double TimeDelta;
        int renderedChunk = 0;

        //检测帧速率
        if (timer() - fctime >= 1.0) {
            fps = fpsc;
            fpsc = 0;
            fctime = timer();
        }
        fpsc++;

        lastframe = curtime;

        if (Player::Running) {
            if (FOVyExt < 9.8) {
                TimeDelta = curtime - SpeedupAnimTimer;
                FOVyExt = 10.0f - (10.0f - FOVyExt) * (float)pow(0.8, TimeDelta * 30);
                SpeedupAnimTimer = curtime;
            }
            else FOVyExt = 10.0;
        }
        else {
            if (FOVyExt > 0.2) {
                TimeDelta = curtime - SpeedupAnimTimer;
                FOVyExt *= (float)pow(0.8, TimeDelta * 30);
                SpeedupAnimTimer = curtime;
            }
            else FOVyExt = 0.0;
        }
        SpeedupAnimTimer = curtime;

        if (Player::OnGround) {
            //半蹲特效
            if (Player::jump < -0.005) {
                if (Player::jump <= -(Player::height - 0.5f))
                    Player::heightExt = -(Player::height - 0.5f);
                else
                    Player::heightExt = (float)Player::jump;
                TouchdownAnimTimer = curtime;
            }
            else {
                if (Player::heightExt <= -0.005) {
                    Player::heightExt *= (float)pow(0.8, (curtime - TouchdownAnimTimer) * 30);
                    TouchdownAnimTimer = curtime;
                }
            }
        }

        double xpos = Player::xpos - Player::xd + (curtime - lastupdate) * 30.0 * Player::xd;
        double ypos = Player::ypos + Player::height + Player::heightExt - Player::yd + (curtime - lastupdate) * 30.0 * Player::
            yd;
        double zpos = Player::zpos - Player::zd + (curtime - lastupdate) * 30.0 * Player::zd;

        if (!bagOpened) {
            //转头！你治好了我多年的颈椎病！
            if (mx != mxl) Player::xlookspeed -= (mx - mxl) * mousemove;
            if (my != myl) Player::ylookspeed += (my - myl) * mousemove;
            if (glfwGetKey(MainWindow, GLFW_KEY_RIGHT) == 1)
                Player::xlookspeed -= mousemove * 16 * (curtime - lastframe
                ) * 30.0;
            if (glfwGetKey(MainWindow, GLFW_KEY_LEFT) == 1)
                Player::xlookspeed += mousemove * 16 * (curtime - lastframe)
                    * 30.0;
            if (glfwGetKey(MainWindow, GLFW_KEY_UP) == 1)
                Player::ylookspeed -= mousemove * 16 * (curtime - lastframe) *
                    30.0;
            if (glfwGetKey(MainWindow, GLFW_KEY_DOWN) == 1)
                Player::ylookspeed += mousemove * 16 * (curtime - lastframe)
                    * 30.0;
            //限制角度，别把头转掉下来了 ←_←
            if (Player::lookupdown + Player::ylookspeed < -90.0) Player::ylookspeed = -90.0 - Player::lookupdown;
            if (Player::lookupdown + Player::ylookspeed > 90.0) Player::ylookspeed = 90.0 - Player::lookupdown;
        }

        Player::cxt = World::getChunkPos((int)Player::xpos);
        Player::cyt = World::getChunkPos((int)Player::ypos);
        Player::czt = World::getChunkPos((int)Player::zpos);

        //更新区块VBO
        World::sortChunkBuildRenderList(lround(Player::xpos), lround(Player::ypos), lround(Player::zpos));
        for (auto&& [dist, cp] : World::chunkBuildRenderList)
            cp->buildRender();

        //删除已卸载区块的VBO
        if (!World::vbuffersShouldDelete.empty()) {
            glDeleteBuffersARB(World::vbuffersShouldDelete.size(), World::vbuffersShouldDelete.data());
            World::vbuffersShouldDelete.clear();
        }

        glFlush();

        double plookupdown = Player::lookupdown + Player::ylookspeed;
        double pheading = Player::heading + Player::xlookspeed;

        glDepthFunc(GL_LEQUAL);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);

        //daylight = clamp((1.0 - cos((double)gametime / gameTimemax * 2.0 * M_PI) * 2.0) / 2.0, 0.05, 1.0);
        //Renderer::sunlightXrot = 90 * daylight;
        if (Renderer::AdvancedRender) {
            //Build shadow map
            if (!DebugShadow) ShadowMaps::BuildShadowMap(xpos, ypos, zpos, curtime);
            else ShadowMaps::RenderShadowMap(xpos, ypos, zpos, curtime);
        }
        glClearColor(skycolorR, skycolorG, skycolorB, 1.0);
        if (!DebugShadow) glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_TEXTURE_2D);

        Player::ViewFrustum.loadIdentity();
        Player::ViewFrustum.setPerspective(FOVyNormal + FOVyExt, (float)windowwidth / windowheight, 0.05f,
                                           viewdistance * 16.0f);
        Player::ViewFrustum.multRotate((float)plookupdown, 1, 0, 0);
        Player::ViewFrustum.multRotate(360.0f - (float)pheading, 0, 1, 0);
        Player::ViewFrustum.update();

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glMultMatrixf(Player::ViewFrustum.getProjMatrix());
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glRotated(plookupdown, 1, 0, 0);
        glRotated(360.0 - pheading, 0, 1, 0);

        World::calcVisible(xpos, ypos, zpos, Player::ViewFrustum);
        WorldRenderer::ListRenderChunks(Player::cxt, Player::cyt, Player::czt, viewdistance, curtime);

        MutexUnlock(Mutex);

        if (MergeFace) {
            glDisable(GL_TEXTURE_2D);
            glEnable(GL_TEXTURE_3D);
            glBindTexture(GL_TEXTURE_3D, BlockTextures3D);
        }
        else glBindTexture(GL_TEXTURE_2D, BlockTextures);

        if (DebugMergeFace) {
            glDisable(GL_LINE_SMOOTH);
            glPolygonMode(GL_FRONT, GL_LINE);
        }

        glDisable(GL_BLEND);

        if (Renderer::AdvancedRender) Renderer::EnableShaders();
        if (!DebugShadow) WorldRenderer::RenderChunks(xpos, ypos, zpos, 0);
        if (Renderer::AdvancedRender) Renderer::DisableShaders();

        glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);

        if (MergeFace) {
            glDisable(GL_TEXTURE_3D);
            glEnable(GL_TEXTURE_2D);
        }
        glEnable(GL_BLEND);

        if (DebugMergeFace) {
            glEnable(GL_LINE_SMOOTH);
            glPolygonMode(GL_FRONT, GL_FILL);
        }

        MutexLock(Mutex);

        if (seldes > 0.0) {
            glTranslated(selx - xpos, sely - ypos, selz - zpos);
            renderDestroy(seldes, 0, 0, 0);
            glTranslated(-selx + xpos, -sely + ypos, -selz + zpos);
        }
        glBindTexture(GL_TEXTURE_2D, BlockTextures);
        Particles::renderall(xpos, ypos, zpos);

        glDisable(GL_TEXTURE_2D);
        if (GUIrenderswitch && sel) {
            glTranslated(selx - xpos, sely - ypos, selz - zpos);
            drawBorder(0, 0, 0);
            glTranslated(-selx + xpos, -sely + ypos, -selz + zpos);
        }

        MutexUnlock(Mutex);

        glLoadIdentity();
        glRotated(plookupdown, 1, 0, 0);
        glRotated(360.0 - pheading, 0, 1, 0);
        glEnable(GL_TEXTURE_2D);
        glEnable(GL_CULL_FACE);

        if (Renderer::AdvancedRender) Renderer::EnableShaders();

        if (MergeFace) {
            glDisable(GL_TEXTURE_2D);
            glEnable(GL_TEXTURE_3D);
            glBindTexture(GL_TEXTURE_3D, BlockTextures3D);
        }
        else glBindTexture(GL_TEXTURE_2D, BlockTextures);

        if (DebugMergeFace) {
            glDisable(GL_LINE_SMOOTH);
            glPolygonMode(GL_FRONT, GL_LINE);
        }

        if (!DebugShadow) WorldRenderer::RenderChunks(xpos, ypos, zpos, 1);
        glDisable(GL_CULL_FACE);
        if (!DebugShadow) WorldRenderer::RenderChunks(xpos, ypos, zpos, 2);
        if (Renderer::AdvancedRender) Renderer::DisableShaders();

        glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);

        if (MergeFace) {
            glDisable(GL_TEXTURE_3D);
            glEnable(GL_TEXTURE_2D);
        }

        if (DebugMergeFace) {
            glEnable(GL_LINE_SMOOTH);
            glPolygonMode(GL_FRONT, GL_FILL);
        }

        glLoadIdentity();
        glRotated(plookupdown, 1, 0, 0);
        glRotated(360.0 - pheading, 0, 1, 0);
        glTranslated(-xpos, -ypos, -zpos);

        MutexLock(Mutex);

        glEnable(GL_CULL_FACE);
        glEnable(GL_TEXTURE_2D);

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(0, windowwidth, windowheight, 0, -1.0, 1.0);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        if (World::getBlock(lround(xpos), lround(ypos), lround(zpos)) == Blocks::WATER) {
            glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
            glBindTexture(GL_TEXTURE_2D, BlockTextures);
            double tcX = Textures::getTexcoordX(Blocks::WATER, 1);
            double tcY = Textures::getTexcoordY(Blocks::WATER, 1);
            glBegin(GL_QUADS);
            glTexCoord2d(tcX, tcY + 1 / 8.0);
            glVertex2i(0, 0);
            glTexCoord2d(tcX, tcY);
            glVertex2i(0, windowheight);
            glTexCoord2d(tcX + 1 / 8.0, tcY);
            glVertex2i(windowwidth, windowheight);
            glTexCoord2d(tcX + 1 / 8.0, tcY + 1 / 8.0);
            glVertex2i(windowwidth, 0);
            glEnd();
        }
        if (GUIrenderswitch) {
            drawGUI();
            drawBag();
        }

        glDisable(GL_TEXTURE_2D);
        if (curtime - screenshotAnimTimer <= 1.0 && !shouldGetScreenshot) {
            float col = 1.0f - (float)(curtime - screenshotAnimTimer);
            glColor4f(1.0f, 1.0f, 1.0f, col);
            glBegin(GL_QUADS);
            glVertex2i(0, 0);
            glVertex2i(0, windowheight);
            glVertex2i(windowwidth, windowheight);
            glVertex2i(windowwidth, 0);
            glEnd();
        }
        glEnable(GL_TEXTURE_2D);

        if (shouldGetScreenshot) {
            shouldGetScreenshot = false;
            screenshotAnimTimer = curtime;
            time_t t = time(nullptr);
            char tmp[64];
            tm timeinfo{};
            localtime_s(&timeinfo, &t);
            strftime(tmp, sizeof tmp, "%Y-%m-%d-%H:%M:%S", &timeinfo);
            std::stringstream ss;
            ss << "Screenshots/" << tmp << ".bmp";
            saveScreenshot(0, 0, windowwidth, windowheight, ss.str());
        }
        if (shouldGetThumbnail) {
            shouldGetThumbnail = false;
            createThumbnail();
        }

        //屏幕刷新，千万别删，后果自负！！！
        //====refresh====//
        MutexUnlock(Mutex);
    }

    void onRender() override {
        MutexLock(Mutex);
        //==refresh end==//
    }

    void drawBorder(int x, int y, int z) {
        //绘制选择边框
        constexpr float eps = 0.002f; //实际上这个边框应该比方块大一些，否则很难看
        glEnable(GL_LINE_SMOOTH);
        glLineWidth(1);
        glColor3f(0.2f, 0.2f, 0.2f);
        glBegin(GL_LINES);
        // Left Face
        glVertex3f(0.5f + eps + x, -(0.5f + eps) + y, -(0.5f + eps) + z);
        glVertex3f(0.5f + eps + x, 0.5f + eps + y, -(0.5f + eps) + z);
        glVertex3f(0.5f + eps + x, 0.5f + eps + y, -(0.5f + eps) + z);
        glVertex3f(0.5f + eps + x, 0.5f + eps + y, 0.5f + eps + z);
        glVertex3f(0.5f + eps + x, 0.5f + eps + y, 0.5f + eps + z);
        glVertex3f(0.5f + eps + x, -(0.5f + eps) + y, 0.5f + eps + z);
        glVertex3f(0.5f + eps + x, -(0.5f + eps) + y, -(0.5f + eps) + z);
        glVertex3f(0.5f + eps + x, -(0.5f + eps) + y, 0.5f + eps + z);
        // Front Face
        glVertex3f(-(0.5f + eps) + x, -(0.5f + eps) + y, 0.5f + eps + z);
        glVertex3f(0.5f + eps + x, -(0.5f + eps) + y, 0.5f + eps + z);
        glVertex3f(0.5f + eps + x, -(0.5f + eps) + y, 0.5f + eps + z);
        glVertex3f(0.5f + eps + x, 0.5f + eps + y, 0.5f + eps + z);
        glVertex3f(0.5f + eps + x, 0.5f + eps + y, 0.5f + eps + z);
        glVertex3f(-(0.5f + eps) + x, 0.5f + eps + y, 0.5f + eps + z);
        glVertex3f(-(0.5f + eps) + x, -(0.5f + eps) + y, 0.5f + eps + z);
        glVertex3f(-(0.5f + eps) + x, 0.5f + eps + y, 0.5f + eps + z);
        // Right Face
        glVertex3f(-(0.5f + eps) + x, -(0.5f + eps) + y, -(0.5f + eps) + z);
        glVertex3f(-(0.5f + eps) + x, -(0.5f + eps) + y, 0.5f + eps + z);
        glVertex3f(-(0.5f + eps) + x, -(0.5f + eps) + y, 0.5f + eps + z);
        glVertex3f(-(0.5f + eps) + x, 0.5f + eps + y, 0.5f + eps + z);
        glVertex3f(-(0.5f + eps) + x, 0.5f + eps + y, 0.5f + eps + z);
        glVertex3f(-(0.5f + eps) + x, 0.5f + eps + y, -(0.5f + eps) + z);
        glVertex3f(-(0.5f + eps) + x, -(0.5f + eps) + y, -(0.5f + eps) + z);
        glVertex3f(-(0.5f + eps) + x, 0.5f + eps + y, -(0.5f + eps) + z);
        // Back Face
        glVertex3f(-(0.5f + eps) + x, -(0.5f + eps) + y, -(0.5f + eps) + z);
        glVertex3f(-(0.5f + eps) + x, 0.5f + eps + y, -(0.5f + eps) + z);
        glVertex3f(-(0.5f + eps) + x, 0.5f + eps + y, -(0.5f + eps) + z);
        glVertex3f(0.5f + eps + x, 0.5f + eps + y, -(0.5f + eps) + z);
        glVertex3f(0.5f + eps + x, 0.5f + eps + y, -(0.5f + eps) + z);
        glVertex3f(0.5f + eps + x, -(0.5f + eps) + y, -(0.5f + eps) + z);
        glVertex3f(-(0.5f + eps) + x, -(0.5f + eps) + y, -(0.5f + eps) + z);
        glVertex3f(0.5f + eps + x, -(0.5f + eps) + y, -(0.5f + eps) + z);
        glEnd();
        glDisable(GL_LINE_SMOOTH);
    }

    void drawGUI() {
        int windowuswidth = windowwidth / stretch, windowusheight = windowheight / stretch;
        glDepthFunc(GL_ALWAYS);
        glDisable(GL_TEXTURE_2D);
        glEnable(GL_LINE_SMOOTH);
        float seldes_100 = seldes / 100.0f;
        int disti = (int)(seldes_100 * linedist);

        if (DebugMode) {
            if (selb != Blocks::AIR) {
                glLineWidth(1);
                glBegin(GL_LINES);
                glColor4f(GUI::FgR, GUI::FgG, GUI::FgB, 0.8f);
                UIVertex(windowuswidth / 2, windowusheight / 2);
                UIVertex(windowuswidth / 2 + 50, windowusheight / 2 + 50);
                UIVertex(windowuswidth / 2 + 50, windowusheight / 2 + 50);
                UIVertex(windowuswidth / 2 + 250, windowusheight / 2 + 50);
                glEnd();
                TextRenderer::setFontColor(1.0f, 1.0f, 1.0f, 0.8f);
                glEnable(GL_TEXTURE_2D);
                glDisable(GL_CULL_FACE);
                std::stringstream ss;
                ss << getBlockInfo(selb).getBlockName() << " (ID " << (int)selb << ")";
                TextRenderer::renderString(windowuswidth / 2 + 50, windowusheight / 2 + 50 - 16, ss.str());
                glDisable(GL_TEXTURE_2D);
                glEnable(GL_CULL_FACE);
                glColor4f(0.0f, 0.0f, 0.0f, 0.9f);
            }
            else { glColor4f(0.0f, 0.0f, 0.0f, 0.6f); }

            glLineWidth(2);

            glBegin(GL_LINES);
            UIVertex(windowuswidth / 2 - linedist + disti, windowusheight / 2 - linedist + disti);
            UIVertex(windowuswidth / 2 - linedist + disti, windowusheight / 2 - linedist + linelength + disti);
            UIVertex(windowuswidth / 2 - linedist + disti, windowusheight / 2 - linedist + disti);
            UIVertex(windowuswidth / 2 - linedist + linelength + disti, windowusheight / 2 - linedist + disti);

            UIVertex(windowuswidth / 2 + linedist - disti, windowusheight / 2 - linedist + disti);
            UIVertex(windowuswidth / 2 + linedist - disti, windowusheight / 2 - linedist + linelength + disti);
            UIVertex(windowuswidth / 2 + linedist - disti, windowusheight / 2 - linedist + disti);
            UIVertex(windowuswidth / 2 + linedist - linelength - disti, windowusheight / 2 - linedist + disti);

            UIVertex(windowuswidth / 2 - linedist + disti, windowusheight / 2 + linedist - disti);
            UIVertex(windowuswidth / 2 - linedist + disti, windowusheight / 2 + linedist - linelength - disti);
            UIVertex(windowuswidth / 2 - linedist + disti, windowusheight / 2 + linedist - disti);
            UIVertex(windowuswidth / 2 - linedist + linelength + disti, windowusheight / 2 + linedist - disti);

            UIVertex(windowuswidth / 2 + linedist - disti, windowusheight / 2 + linedist - disti);
            UIVertex(windowuswidth / 2 + linedist - disti, windowusheight / 2 + linedist - linelength - disti);
            UIVertex(windowuswidth / 2 + linedist - disti, windowusheight / 2 + linedist - disti);
            UIVertex(windowuswidth / 2 + linedist - linelength - disti, windowusheight / 2 + linedist - disti);

            glEnd();
        }

        glLineWidth(4 * stretch);
        glBegin(GL_LINES);
        glColor4f(0.0, 0.0, 0.0, 1.0);
        UIVertex(windowuswidth / 2 - 16, windowusheight / 2);
        UIVertex(windowuswidth / 2 + 16, windowusheight / 2);
        UIVertex(windowuswidth / 2, windowusheight / 2 - 16);
        UIVertex(windowuswidth / 2, windowusheight / 2 + 16);
        glEnd();
        glLineWidth(2 * stretch);
        glBegin(GL_LINES);
        glColor4f(1.0, 1.0, 1.0, 1.0);
        UIVertex(windowuswidth / 2 - 15, windowusheight / 2);
        UIVertex(windowuswidth / 2 + 15, windowusheight / 2);
        UIVertex(windowuswidth / 2, windowusheight / 2 - 15);
        UIVertex(windowuswidth / 2, windowusheight / 2 + 15);
        glEnd();

        if (seldes > 0.0) {
            glBegin(GL_LINES);
            glColor4f(0.5, 0.5, 0.5, 1.0);
            glVertex2i(windowwidth / 2 - 15, windowheight / 2);
            glVertex2i(windowwidth / 2 - 15 + (int)(seldes_100 * 15), windowheight / 2);
            glVertex2i(windowwidth / 2 + 15, windowheight / 2);
            glVertex2i(windowwidth / 2 + 15 - (int)(seldes_100 * 15), windowheight / 2);
            glVertex2i(windowwidth / 2, windowheight / 2 - 15);
            glVertex2i(windowwidth / 2, windowheight / 2 - 15 + (int)(seldes_100 * 15));
            glVertex2i(windowwidth / 2, windowheight / 2 + 15);
            glVertex2i(windowwidth / 2, windowheight / 2 + 15 - (int)(seldes_100 * 15));
            glEnd();
        }

        glDisable(GL_CULL_FACE);

        if (Player::gamemode == Player::Survival) {
            glColor4d(0.8, 0.0, 0.0, 0.3);
            glBegin(GL_QUADS);
            UIVertex(10, 10);
            UIVertex(200, 10);
            UIVertex(200, 30);
            UIVertex(10, 30);
            glEnd();

            double healthPercent = (double)Player::health / Player::healthmax;
            glColor4d(1.0, 0.0, 0.0, 0.5);
            glBegin(GL_QUADS);
            UIVertex(20, 15);
            UIVertex(static_cast<int>(20 + healthPercent * 170), 15);
            UIVertex(static_cast<int>(20 + healthPercent * 170), 25);
            UIVertex(20, 25);
            glEnd();
        }

        TextRenderer::setFontColor(1.0f, 1.0f, 1.0f, 0.9f);
        if (chatmode) {
            glColor4f(GUI::FgR, GUI::FgG, GUI::FgB, GUI::FgA);
            glDisable(GL_TEXTURE_2D);
            glBegin(GL_QUADS);
            glVertex2i(1, windowheight - 33);
            glVertex2i(windowwidth - 1, windowheight - 33);
            glVertex2i(windowwidth - 1, windowheight - 51);
            glVertex2i(1, windowheight - 51);
            glEnd();
            glEnable(GL_TEXTURE_2D);
            TextRenderer::renderString(0, windowheight - 50, chatword);
        }
        int posy = 0;
        int size = chatMessages.size();
        if (size != 0) {
            for (int i = size - 1; i >= (size - 10 > 0 ? size - 10 : 0); --i) {
                TextRenderer::renderString(0, windowheight - 80 - 18 * posy++, chatMessages[i]);
            }
        }

        //if (DebugShadow) ShadowMaps::DrawShadowMap(windowwidth / 2, windowheight / 2, windowwidth, windowheight);

        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, TextRenderer::Font);

        if (DebugMode) {
            std::stringstream ss;
            //ss << std::fixed << std::setprecision(4);
            ss << "NEWorld v" << VERSION << " [OpenGL " << GLVersionMajor << "." << GLVersionMinor << "|" <<
                GLVersionRev << "]";
            debugText(ss.str(), false);
            ss.str("");
            ss << "Fps:" << fps << "|" << "Ups:" << ups;
            debugText(ss.str(), false);
            ss.str("");

            ss << "Debug Mode:" << std::boolalpha << DebugMode;
            debugText(ss.str(), false);
            ss.str("");
            if (Renderer::AdvancedRender) {
                ss << "Shadow View:" << std::boolalpha << DebugShadow;
                debugText(ss.str(), false);
                ss.str("");
            }
            ss << "X:" << Player::xpos << " Y:" << Player::ypos << " Z:" << Player::zpos;
            debugText(ss.str(), false);
            ss.str("");
            ss << "Direction:" << Player::heading << " Head:" << Player::lookupdown << "Jump speed:" << Player::jump;
            debugText(ss.str(), false);
            ss.str("");

            ss << "Stats:";
            if (Player::Flying) ss << " Flying";
            if (Player::OnGround) ss << " On_ground";
            if (Player::NearWall) ss << " Near_wall";
            if (Player::inWater) ss << " In_water";
            if (Player::CrossWall) ss << " Cross_Wall";
            if (Player::Glide) ss << " Gliding_enabled";
            if (Player::glidingNow) ss << "Gliding";
            debugText(ss.str(), false);
            ss.str("");

            ss << "Energy:" << Player::glidingEnergy;
            debugText(ss.str(), false);
            ss.str("");
            ss << "Speed:" << Player::glidingSpeed;
            debugText(ss.str(), false);
            ss.str("");

            int h = gametime / (30 * 60);
            int m = gametime % (30 * 60) / 30;
            int s = gametime % 30 * 2;
            ss << "Time: "
                << (h < 10 ? "0" : "") << h << ":"
                << (m < 10 ? "0" : "") << m << ":"
                << (s < 10 ? "0" : "") << s
                << " (" << gametime << "/" << gameTimemax << ")";
            debugText(ss.str(), false);
            ss.str("");

            ss << "load:" << World::loadedChunks << " unload:" << World::unloadedChunks
                << " render:" << WorldRenderer::RenderChunkList.size() << " update:" << World::updatedChunks;
            debugText(ss.str(), false);
            ss.str("");
            debugText("", true);
        }
        else {
            TextRenderer::setFontColor(1.0f, 1.0f, 1.0f, 0.9f);
            std::stringstream ss;
            ss << "v" << VERSION << "  Fps:" << fps;
            TextRenderer::renderString(10, 30, ss.str());
        }
        glFlush();
    }

    void drawCloud(double px, double pz) {
        //glFogf(GL_FOG_START, 100.0);
        //glFogf(GL_FOG_END, 300.0);
        static double ltimer;
        static unsigned int cloudvb[128];
        static int vtxs[128];
        static float f;
        static int l;
        if (ltimer == 0.0) ltimer = timer();
        f += (float)(timer() - ltimer) * 0.25f;
        ltimer = timer();
        if (f >= 1.0) {
            l += int(f);
            f -= int(f);
            l %= 128;
        }

        /*if (!generated) {
            generated = true;
            for (int i = 0; i != 128; i++) { for (int j = 0; j != 128; j++) { World::cloud[i][j] = int(rnd() * 2); } }
            glGenBuffersARB(128, cloudvb);
            for (int i = 0; i != 128; i++) {
                Renderer::Init(0, 0);
                for (int j = 0; j != 128; j++) {
                    if (World::cloud[i][j] != 0) {
                        Renderer::Vertex3d(j * cloudwidth, 128.0, 0.0);
                        Renderer::Vertex3d(j * cloudwidth, 128.0, cloudwidth);
                        Renderer::Vertex3d((j + 1) * cloudwidth, 128.0, cloudwidth);
                        Renderer::Vertex3d((j + 1) * cloudwidth, 128.0, 0.0);
                    }
                }
                Renderer::Flush(cloudvb[i], vtxs[i]);
            }
        }

        glDisable(GL_TEXTURE_2D);
        glDisable(GL_CULL_FACE);
        glColor4f(1.0, 1.0, 1.0, 0.5);
        for (int i = 0; i < 128; i++) {
            glPushMatrix();
            glTranslated(-64.0 * cloudwidth - px, 0.0, cloudwidth * ((l + i) % 128 + f) - 64.0 * cloudwidth - pz);
            Renderer::renderbuffer(cloudvb[i], vtxs[i], 0, 0);
            glPopMatrix();
        }*/
        //setupNormalFog();
    }

    void renderDestroy(float level, int x, int y, int z) {
        static float eps = 0.002f;
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

        if (level < 100.0) glBindTexture(GL_TEXTURE_2D, DestroyImage[int(level / 10) + 1]);
        else glBindTexture(GL_TEXTURE_2D, DestroyImage[10]);

        glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 0.0f);
        glVertex3f(-(0.5f + eps) + x, -(0.5f + eps) + y, 0.5f + eps + z);
        glTexCoord2f(1.0f, 0.0f);
        glVertex3f(0.5f + eps + x, -(0.5f + eps) + y, 0.5f + eps + z);
        glTexCoord2f(1.0f, 1.0f);
        glVertex3f(0.5f + eps + x, 0.5f + eps + y, 0.5f + eps + z);
        glTexCoord2f(0.0f, 1.0f);
        glVertex3f(-(0.5f + eps) + x, 0.5f + eps + y, 0.5f + eps + z);

        glTexCoord2f(1.0f, 0.0f);
        glVertex3f(-(0.5f + eps) + x, -(0.5f + eps) + y, -(0.5f + eps) + z);
        glTexCoord2f(1.0f, 1.0f);
        glVertex3f(-(0.5f + eps) + x, 0.5f + eps + y, -(0.5f + eps) + z);
        glTexCoord2f(0.0f, 1.0f);
        glVertex3f(0.5f + eps + x, 0.5f + eps + y, -(0.5f + eps) + z);
        glTexCoord2f(0.0f, 0.0f);
        glVertex3f(0.5f + eps + x, -(0.5f + eps) + y, -(0.5f + eps) + z);

        glTexCoord2f(1.0f, 0.0f);
        glVertex3f(0.5f + eps + x, -(0.5f + eps) + y, -(0.5f + eps) + z);
        glTexCoord2f(1.0f, 1.0f);
        glVertex3f(0.5f + eps + x, 0.5f + eps + y, -(0.5f + eps) + z);
        glTexCoord2f(0.0f, 1.0f);
        glVertex3f(0.5f + eps + x, 0.5f + eps + y, 0.5f + eps + z);
        glTexCoord2f(0.0f, 0.0f);
        glVertex3f(0.5f + eps + x, -(0.5f + eps) + y, 0.5f + eps + z);

        glTexCoord2f(0.0f, 0.0f);
        glVertex3f(-(0.5f + eps) + x, -(0.5f + eps) + y, -(0.5f + eps) + z);
        glTexCoord2f(1.0f, 0.0f);
        glVertex3f(-(0.5f + eps) + x, -(0.5f + eps) + y, 0.5f + eps + z);
        glTexCoord2f(1.0f, 1.0f);
        glVertex3f(-(0.5f + eps) + x, 0.5f + eps + y, 0.5f + eps + z);
        glTexCoord2f(0.0f, 1.0f);
        glVertex3f(-(0.5f + eps) + x, 0.5f + eps + y, -(0.5f + eps) + z);

        glTexCoord2f(0.0f, 1.0f);
        glVertex3f(-(0.5f + eps) + x, 0.5f + eps + y, -(0.5f + eps) + z);
        glTexCoord2f(0.0f, 0.0f);
        glVertex3f(-(0.5f + eps) + x, 0.5f + eps + y, 0.5f + eps + z);
        glTexCoord2f(1.0f, 0.0f);
        glVertex3f(0.5f + eps + x, 0.5f + eps + y, 0.5f + eps + z);
        glTexCoord2f(1.0f, 1.0f);
        glVertex3f(0.5f + eps + x, 0.5f + eps + y, -(0.5f + eps) + z);

        glTexCoord2f(1.0f, 1.0f);
        glVertex3f(-(0.5f + eps) + x, -(0.5f + eps) + y, -(0.5f + eps) + z);
        glTexCoord2f(0.0f, 1.0f);
        glVertex3f(0.5f + eps + x, -(0.5f + eps) + y, -(0.5f + eps) + z);
        glTexCoord2f(0.0f, 0.0f);
        glVertex3f(0.5f + eps + x, -(0.5f + eps) + y, 0.5f + eps + z);
        glTexCoord2f(1.0f, 0.0f);
        glVertex3f(-(0.5f + eps) + x, -(0.5f + eps) + y, 0.5f + eps + z);
        glEnd();
    }

    void drawBagRow(int row, int itemid, int xbase, int ybase, int spac, float alpha) {
        //画出背包的一行
        for (int i = 0; i < 10; i++) {
            if (i == itemid) glBindTexture(GL_TEXTURE_2D, tex_select);
            else glBindTexture(GL_TEXTURE_2D, tex_unselect);
            glColor4f(1.0f, 1.0f, 1.0f, alpha);
            glBegin(GL_QUADS);
            glTexCoord2f(0.0, 1.0);
            UIVertex(xbase + i * (32 + spac), ybase);
            glTexCoord2f(0.0, 0.0);
            UIVertex(xbase + i * (32 + spac) + 32, ybase);
            glTexCoord2f(1.0, 0.0);
            UIVertex(xbase + i * (32 + spac) + 32, ybase + 32);
            glTexCoord2f(1.0, 1.0);
            UIVertex(xbase + i * (32 + spac), ybase + 32);
            glEnd();
            glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
            if (Player::inventory[row][i] != Blocks::AIR) {
                glBindTexture(GL_TEXTURE_2D, BlockTextures);
                double tcX = Textures::getTexcoordX(Player::inventory[row][i], 1);
                double tcY = Textures::getTexcoordY(Player::inventory[row][i], 1);
                glBegin(GL_QUADS);
                glTexCoord2d(tcX, tcY + 1 / 8.0);
                UIVertex(xbase + i * (32 + spac) + 2, ybase + 2);
                glTexCoord2d(tcX + 1 / 8.0, tcY + 1 / 8.0);
                UIVertex(xbase + i * (32 + spac) + 30, ybase + 2);
                glTexCoord2d(tcX + 1 / 8.0, tcY);
                UIVertex(xbase + i * (32 + spac) + 30, ybase + 30);
                glTexCoord2d(tcX, tcY);
                UIVertex(xbase + i * (32 + spac) + 2, ybase + 30);
                glEnd();
                std::stringstream ss;
                ss << (int)Player::inventoryAmount[row][i];
                TextRenderer::renderString(xbase + i * (32 + spac), ybase, ss.str());
            }
        }
    }

    void drawBag() {
        //背包界面与更新
        static int si, sj, sf;
        int csi = -1, csj = -1;
        int leftp = (windowwidth / stretch - 392) / 2;
        int upp = windowheight / stretch - 152 - 16;
        static int mousew, mouseb, mousebl;
        static Block indexselected = Blocks::AIR;
        static short Amountselected = 0;
        double curtime = timer();
        double TimeDelta = curtime - bagAnimTimer;
        float bagAnim = (float)(1.0 - pow(0.9, TimeDelta * 60.0) + pow(0.9, bagAnimDuration * 60.0) / bagAnimDuration *
            TimeDelta);

        if (bagOpened) {
            glfwSetInputMode(MainWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            mousew = mw;
            mouseb = mb;
            glDepthFunc(GL_ALWAYS);
            glDisable(GL_CULL_FACE);
            glDisable(GL_TEXTURE_2D);

            if (curtime - bagAnimTimer > bagAnimDuration) glColor4f(0.2f, 0.2f, 0.2f, 0.6f);
            else glColor4f(0.2f, 0.2f, 0.2f, 0.6f * bagAnim);
            glBegin(GL_QUADS);
            UIVertex(0, 0);
            UIVertex((int)(windowwidth / stretch), 0);
            UIVertex((int)(windowwidth / stretch), (int)(windowheight / stretch));
            UIVertex(0, (int)(windowheight / stretch));
            glEnd();

            glEnable(GL_TEXTURE_2D);
            glDisable(GL_CULL_FACE);
            glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
            sf = 0;

            if (curtime - bagAnimTimer > bagAnimDuration) {
                for (int i = 0; i < 4; i++) {
                    for (int j = 0; j < 10; j++) {
                        if (mx >= j * 40 + leftp && mx <= j * 40 + 32 + leftp &&
                            my >= i * 40 + upp && my <= i * 40 + 32 + upp) {
                            csi = si = i;
                            csj = sj = j;
                            sf = 1;
                            if (mousebl == 0 && mouseb == 1 && indexselected == Player::inventory[i][j]) {
                                if (Player::inventoryAmount[i][j] + Amountselected <= 255) {
                                    Player::inventoryAmount[i][j] += Amountselected;
                                    Amountselected = 0;
                                }
                                else {
                                    Amountselected = Player::inventoryAmount[i][j] + Amountselected - 255;
                                    Player::inventoryAmount[i][j] = 255;
                                }
                            }
                            if (mousebl == 0 && mouseb == 1 && indexselected != Player::inventory[i][j]) {
                                std::swap(Amountselected, Player::inventoryAmount[i][j]);
                                std::swap(indexselected, Player::inventory[i][j]);
                            }
                            if (mousebl == 0 && mouseb == 2 && indexselected == Player::inventory[i][j] && Player::
                                inventoryAmount[i][j] < 255) {
                                Amountselected--;
                                Player::inventoryAmount[i][j]++;
                            }
                            if (mousebl == 0 && mouseb == 2 && Player::inventory[i][j] == Blocks::AIR) {
                                Amountselected--;
                                Player::inventoryAmount[i][j] = 1;
                                Player::inventory[i][j] = indexselected;
                            }

                            if (Amountselected == 0) indexselected = Blocks::AIR;
                            if (indexselected == Blocks::AIR) Amountselected = 0;
                            if (Player::inventoryAmount[i][j] == 0) Player::inventory[i][j] = Blocks::AIR;
                            if (Player::inventory[i][j] == Blocks::AIR) Player::inventoryAmount[i][j] = 0;
                        }
                    }
                    drawBagRow(i, csi == i ? csj : -1, (windowwidth / stretch - 392) / 2,
                               windowheight / stretch - 152 - 16 + i * 40, 8, 1.0f);
                }
            }
            if (indexselected != Blocks::AIR) {
                glBindTexture(GL_TEXTURE_2D, BlockTextures);
                double tcX = Textures::getTexcoordX(indexselected, 1);
                double tcY = Textures::getTexcoordY(indexselected, 1);
                glBegin(GL_QUADS);
                glTexCoord2d(tcX, tcY + 1 / 8.0);
                UIVertex(mx - 16, my - 16);
                glTexCoord2d(tcX + 1 / 8.0, tcY + 1 / 8.0);
                UIVertex(mx + 16, my - 16);
                glTexCoord2d(tcX + 1 / 8.0, tcY);
                UIVertex(mx + 16, my + 16);
                glTexCoord2d(tcX, tcY);
                UIVertex(mx - 16, my + 16);
                glEnd();
                std::stringstream ss;
                ss << Amountselected;
                TextRenderer::renderString((int)mx - 16, (int)my - 16, ss.str());
            }
            if (Player::inventory[si][sj] != 0 && sf == 1) {
                glColor4f(1.0, 1.0, 0.0, 1.0);
                TextRenderer::renderString((int)mx, (int)my - 16,
                                           getBlockInfo(Player::inventory[si][sj]).getBlockName());
            }

            float alpha = 0.5f + 0.5f * bagAnim;
            if (curtime - bagAnimTimer <= bagAnimDuration) {
                int xbase = (int)round((windowwidth / stretch - 392) / 2 * bagAnim);
                int ybase = (int)round(
                    (windowheight / stretch - 152 - 16 + 120 - (windowheight / stretch - 32)) * bagAnim + (windowheight
                        / stretch - 32));
                int spac = (int)round(8 * bagAnim);
                drawBagRow(3, -1, xbase, ybase, spac, alpha);
                xbase = (int)round(
                    ((windowwidth / stretch - 392) / 2 - windowwidth / stretch) * bagAnim + windowwidth / stretch);
                ybase = (int)round(
                    (windowheight / stretch - 152 - 16 - (windowheight / stretch - 32)) * bagAnim + (windowheight /
                        stretch - 32));
                for (int i = 0; i < 3; i++) {
                    glColor4f(1.0f, 1.0f, 1.0f, bagAnim);
                    drawBagRow(i, -1, xbase, ybase + i * 40, spac, alpha);
                }
            }

            glEnable(GL_TEXTURE_2D);
            glDisable(GL_CULL_FACE);
            glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

            mousebl = mouseb;
        }
        else {
            glEnable(GL_TEXTURE_2D);
            glDisable(GL_CULL_FACE);
            if (curtime - bagAnimTimer <= bagAnimDuration) {
                glDisable(GL_TEXTURE_2D);
                glColor4f(0.2f, 0.2f, 0.2f, 0.6f - 0.6f * bagAnim);
                glBegin(GL_QUADS);
                glVertex2i(0, 0);
                glVertex2i(windowwidth, 0);
                glVertex2i(windowwidth, windowheight);
                glVertex2i(0, windowheight);
                glEnd();
                glEnable(GL_TEXTURE_2D);
                int xbase = 0, ybase = 0, spac = 0;
                float alpha = 1.0f - 0.5f * bagAnim;
                xbase = (int)round((windowwidth / stretch - 392) / 2 - (windowwidth / stretch - 392) / 2 * bagAnim);
                ybase = (int)round(
                    windowheight / stretch - 152 - 16 + 120 - (windowheight / stretch - 32) - (windowheight / stretch
                        - 152 - 16 + 120 - (windowheight - 32)) * bagAnim + (windowheight / stretch - 32));
                spac = (int)round(8 - 8 * bagAnim);
                glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
                drawBagRow(3, Player::indexInHand, xbase, ybase, spac, alpha);
                xbase = (int)round(
                    (windowwidth / stretch - 392) / 2 - windowwidth / stretch - ((windowwidth / stretch - 392) / 2 -
                        windowwidth / stretch) * bagAnim + windowwidth / stretch);
                ybase = (int)round(
                    windowheight / stretch - 152 - 16 - (windowheight / stretch - 32) - (windowheight / stretch - 152
                        - 16 - (windowheight / stretch - 32)) * bagAnim + (windowheight / stretch - 32));
                for (int i = 0; i < 3; i++) {
                    glColor4f(1.0f, 1.0f, 1.0f, 1.0f - bagAnim);
                    drawBagRow(i, -1, xbase, ybase + i * 40, spac, alpha);
                }
            }
            else drawBagRow(3, Player::indexInHand, 0, windowheight / stretch - 32, 0, 0.5f);
        }
        glFlush();
    }

    void saveScreenshot(int x, int y, int w, int h, const std::string& filename) {
        Textures::TEXTURE_RGB scrBuffer;
        int bufw = w, bufh = h;
        while (bufw % 4 != 0) { bufw += 1; }
        while (bufh % 4 != 0) { bufh += 1; }
        scrBuffer.sizeX = bufw;
        scrBuffer.sizeY = bufh;
        scrBuffer.buffer = std::unique_ptr<uint8_t[]>(new uint8_t[bufw * bufh * 3]);
        glReadPixels(x, y, bufw, bufh, GL_RGB, GL_UNSIGNED_BYTE, scrBuffer.buffer.get());
        SaveRGBImage(filename, scrBuffer);
    }

    void createThumbnail() {
        std::stringstream ss;
        ss << "Worlds/" << World::worldname << "/Thumbnail.bmp";
        saveScreenshot(0, 0, windowwidth, windowheight, ss.str());
    }


    void registerCommands() {
        commands.emplace_back("/give", [](const std::vector<std::string>& command) {
            if (command.size() != 3) return false;
            item itemid;
            conv(command[1], itemid);
            short amount;
            conv(command[2], amount);
            Player::addItem(itemid, amount);
            return true;
        });
        commands.emplace_back("/tp", [](const std::vector<std::string>& command) {
            if (command.size() != 4) return false;
            double x;
            conv(command[1], x);
            double y;
            conv(command[2], y);
            double z;
            conv(command[3], z);
            Player::xpos = x;
            Player::ypos = y;
            Player::zpos = z;
            return true;
        });
        commands.emplace_back("/suicide", [](const std::vector<std::string>& command) {
            if (command.size() != 1) return false;
            Player::spawn();
            return true;
        });
        commands.emplace_back("/setblock", [](const std::vector<std::string>& command) {
            if (command.size() != 5) return false;
            int x;
            conv(command[1], x);
            int y;
            conv(command[2], y);
            int z;
            conv(command[3], z);
            Block b;
            conv(command[4], b);
            World::setblock(x, y, z, b);
            return true;
        });
        commands.emplace_back("/tree", [](const std::vector<std::string>& command) {
            if (command.size() != 4) return false;
            int x;
            conv(command[1], x);
            int y;
            conv(command[2], y);
            int z;
            conv(command[3], z);
            World::buildtree(x, y, z);
            return true;
        });
        commands.emplace_back("/explode", [](const std::vector<std::string>& command) {
            if (command.size() != 5) return false;
            int x;
            conv(command[1], x);
            int y;
            conv(command[2], y);
            int z;
            conv(command[3], z);
            int r;
            conv(command[4], r);
            World::explode(x, y, z, r);
            return true;
        });
        commands.emplace_back("/gamemode", [](const std::vector<std::string>& command) {
            if (command.size() != 2) return false;
            int mode;
            conv(command[1], mode);
            Player::changeGameMode(mode);
            return true;
        });
        commands.emplace_back("/kit", [](const std::vector<std::string>& command) {
            if (command.size() != 1) return false;
            Player::inventory[0][0] = 1;
            Player::inventoryAmount[0][0] = 255;
            Player::inventory[0][1] = 2;
            Player::inventoryAmount[0][1] = 255;
            Player::inventory[0][2] = 3;
            Player::inventoryAmount[0][2] = 255;
            Player::inventory[0][3] = 4;
            Player::inventoryAmount[0][3] = 255;
            Player::inventory[0][4] = 5;
            Player::inventoryAmount[0][4] = 255;
            Player::inventory[0][5] = 6;
            Player::inventoryAmount[0][5] = 255;
            Player::inventory[0][6] = 7;
            Player::inventoryAmount[0][6] = 255;
            Player::inventory[0][7] = 8;
            Player::inventoryAmount[0][7] = 255;
            Player::inventory[0][8] = 9;
            Player::inventoryAmount[0][8] = 255;
            Player::inventory[0][9] = 10;
            Player::inventoryAmount[0][9] = 255;
            Player::inventory[1][0] = 11;
            Player::inventoryAmount[1][0] = 255;
            Player::inventory[1][1] = 12;
            Player::inventoryAmount[1][1] = 255;
            Player::inventory[1][2] = 13;
            Player::inventoryAmount[1][2] = 255;
            Player::inventory[1][3] = 14;
            Player::inventoryAmount[1][3] = 255;
            Player::inventory[1][4] = 15;
            Player::inventoryAmount[1][4] = 255;
            Player::inventory[1][5] = 16;
            Player::inventoryAmount[1][5] = 255;
            Player::inventory[1][6] = 17;
            Player::inventoryAmount[1][6] = 255;
            Player::inventory[1][7] = 18;
            Player::inventoryAmount[1][7] = 255;
            return true;
        });
        commands.emplace_back("/time", [](const std::vector<std::string>& command) {
            if (command.size() != 2) return false;
            int time;
            conv(command[1], time);
            if (time < 0 || time > gameTimemax) return false;
            gametime = time;
            return true;
        });
    }

    bool doCommand(const std::vector<std::string>& command) {
        for (unsigned int i = 0; i != commands.size(); i++) {
            if (command[0] == commands[i].identifier) { return commands[i].execute(command); }
        }
        return false;
    }

    void onLoad() override {
        Background = nullptr;

        glEnable(GL_LINE_SMOOTH);
        glEnable(GL_TEXTURE_2D);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glfwSwapBuffers(MainWindow);
        glfwPollEvents();

        Mutex = MutexCreate();
        MutexLock(Mutex);
        updateThread = ThreadCreate(&updateThreadFunc, nullptr);
        if (multiplayer) {
            fastSrand(static_cast<unsigned int>(time(nullptr)));
            Player::name = "";
            Player::onlineID = rand(); /*
            Network::init(serverip, port);*/
        }
        //初始化游戏状态
        printf("[Console][Game]Init player...\n");
        if (loadGame()) Player::init(Player::xpos, Player::ypos, Player::zpos);
        else Player::spawn();
        printf("[Console][Game]Init world...\n");
        World::init();
        registerCommands();
        printf("[Console][Game]Loading Mods...\n");

        GUIrenderswitch = true;
        glDepthFunc(GL_LEQUAL);
        glEnable(GL_CULL_FACE);
        setupNormalFog();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glfwSwapBuffers(MainWindow);
        glfwPollEvents();
        printf("[Console][Game]Game start!\n");

        //这才是游戏开始!
        glfwSetInputMode(MainWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        mxl = mx;
        myl = my;
        printf("[Console][Game]Main loop started\n");
        updateThreadRun = true;
        fctime = uctime = lastupdate = timer();
    }

    void onUpdate() override {
        MutexUnlock(Mutex);
        MutexLock(Mutex);

        if (timer() - uctime >= 1.0) {
            uctime = timer();
            ups = upsc;
            upsc = 0;
        }

        Grender();

        if (glfwGetKey(MainWindow, GLFW_KEY_ESCAPE) == 1) {
            updateThreadPaused = true;
            createThumbnail();
            GUI::clearTransition();
            Menus::gamemenu();
        }
    }

    void onLeave() override {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        TextRenderer::setFontColor(1.0, 1.0, 1.0, 1.0);
        TextRenderer::renderString(0, 0, "Saving world...");
        glfwSwapBuffers(MainWindow);
        glfwPollEvents();
        printf("[Console][Game]Terminate threads\n");
        updateThreadRun = false;
        MutexUnlock(Mutex);
        ThreadWait(updateThread);
        ThreadDestroy(updateThread);
        MutexDestroy(Mutex);
        saveGame();
        World::destroyAllChunks();
        if (!World::vbuffersShouldDelete.empty()) {
            glDeleteBuffersARB(World::vbuffersShouldDelete.size(), World::vbuffersShouldDelete.data());
            World::vbuffersShouldDelete.clear();
        } /*
        if (multiplayer) Network::cleanUp();*/
        commands.clear();
        chatMessages.clear();
        GUI::BackToMain();
    }
};

GameDView* Game = nullptr;
void GameView() { PushPage(Game = new GameDView); }

ThreadFunc updateThreadFunc(void*) {
    Game->GameThreadloop();
    return 0;
}
