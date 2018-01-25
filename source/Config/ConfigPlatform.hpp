// 
// NWShared: ConfigPlatform.hpp
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
#if defined (__linux__) || defined (linux) || defined (__linux)
#   define NW_TARGET_POSIX
#   define NW_TARGET_LINUX
#elif defined (__APPLE__)
#   define NW_TARGET_POSIX
#   define NW_TARGET_APPLE
#   if defined(__MACH__)
#       define NW_TARGET_MACOSX
#   else
#       define NW_TARGET_IOS
#   endif
#elif defined (_WIN32) || defined (_WIN64) || defined (__WIN32__) || defined (__TOS_WIN__) || defined (__WINDOWS__) || defined (__CYGWIN__)
#   define NW_TARGET_WINDOWS
#endif
