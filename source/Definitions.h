// 
// NEWorld: Definitions.h
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
//#pragma comment(linker, "/SUBSYSTEM:\"WINDOWS\" /ENTRY:\"mainCRTStartup\"")
#pragma comment(linker, "/SUBSYSTEM:\"CONSOLE\" /ENTRY:\"mainCRTStartup\"")
#include "StdInclude.h"
#include "Typedefs.h"
#include "FunctionsKit.h"

//Global Vars
const unsigned int VERSION = 39;
const std::string MAJOR_VERSION = "Alpha 0.";
const std::string minOR_VERSION = "5";
const std::string EXT_VERSION = " Technical Perview";
const int defaultwindowwidth = 852; //默认窗口宽度
const int defaultwindowheight = 480; //默认窗口高度
const int networkRequestFrequency = 3; //请求频率
const int networkRequestmax = 20; //理想最大请求队列长度

inline float FOVyNormal = 60.0f; //视野角度
inline float mousemove = 0.2f; //鼠标灵敏度
inline int viewdistance = 8; //视野距离
inline int cloudwidth = 10; //云的宽度
inline int selectPrecision = 32; //选择方块的精度
inline int selectDistance = 8; //选择方块的距离
inline float walkspeed = 0.15f; //玩家前进速度
inline float runspeed = 0.3f; //玩家跑步速度
inline int maxAirJumps = 3 - 1; //空中N段连跳
inline bool SmoothLighting = true; //平滑光照
inline bool NiceGrass = true; //草地材质连接
inline bool MergeFace = false; //合并面渲染
inline bool GUIScreenBlur = false; //GUI背景模糊  Void:这个算法慢死了，我关掉了  qiaozhanrong:23333我也想关掉
inline int linelength = 10; //跟F3的准星有关。。。
inline int linedist = 30; //跟F3的准星有关。。。
inline bool ppistretch = false; //试验功能，默认关闭
inline float skycolorR = 0.7f; //天空颜色Red
inline float skycolorG = 1.0f; //天空颜色Green
inline float skycolorB = 1.0f; //天空颜色Blue
inline float FOVyRunning = 8.0f;
inline float FOVyExt;
inline double stretch = 1.0f; //ppi缩放比例（供gui绘制使用）
inline int Multisample = 0; //多重采样抗锯齿
inline bool vsync = false; //垂直同步
inline int gametime = 0; //游戏时间 0~2592000
constexpr int gameTimemax = 2592000;
//float daylight;

inline int windowwidth; //窗口宽度
inline int windowheight; //窗口宽度
inline bool gamebegin, gameexit, bagOpened;

//多人游戏
inline bool multiplayer = false;
inline std::string serverip;
inline unsigned short port = 30001;

inline TextureID BlockTextures, BlockTextures3D;
inline TextureID tex_select, tex_unselect, tex_title, tex_mainmenu[6];
inline TextureID DestroyImage[11];
inline TextureID DefaultSkin;

//线程
inline Mutex_t Mutex;
inline Thread_t updateThread;
inline double lastupdate, updateTimer;
inline double lastframe;
inline bool updateThreadRun, updateThreadPaused;

inline bool shouldGetScreenshot;
inline bool shouldGetThumbnail;
inline bool FirstUpdateThisFrame;
inline bool FirstFrameThisUpdate;
inline double SpeedupAnimTimer;
inline double TouchdownAnimTimer;
inline double screenshotAnimTimer;
inline double bagAnimTimer;
inline double bagAnimDuration = 0.5;

//OpenGL
inline int GLVersionMajor, GLVersionMinor, GLVersionRev;
//GLFW
inline GLFWwindow* MainWindow;
inline GLFWcursor* MouseCursor;

//鼠标输入数据
inline double mx, my, mxl, myl;
inline int mw, mb, mbp, mbl, mwl;
inline double mxdelta, mydelta;
//键盘输入数据
inline std::string inputstr;

void AppCleanUp();
