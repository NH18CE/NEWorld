// 
// NWShared: ConfigCompilerSlang.hpp
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
#if defined(_WIN32) || defined(__CYGWIN__)
#    ifdef __GNUC__
#        define EXPORT __attribute__ ((dllexport))
#        define IMPORT __attribute__ ((dllimport))
#    else
#        define EXPORT __declspec(dllexport)
#        define IMPORT __declspec(dllimport)
#    endif
#else
#    define EXPORT __attribute__ ((visibility ("default")))
#    define IMPORT
#endif

#if defined(_MSC_VER)
#pragma warning(disable: 4251)
#pragma warning(disable: 4275)
#define _ENABLE_ATOMIC_ALIGNMENT_FIX
#endif

#ifdef NW_EXPORTS
#define NW_API EXPORT
#else
#define NW_API IMPORT
#endif
