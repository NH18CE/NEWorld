// 
// NWShared: Math.h
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

#include <cmath>
#include <cstdint>

constexpr int8_t roundUptoPowerOf2(int8_t x) noexcept {
    x--;
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x++;
    return x;
}

constexpr int16_t roundUptoPowerOf2(int16_t x) noexcept {
    x--;
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x++;
    return x;
}

constexpr int32_t roundUptoPowerOf2(int32_t x) noexcept {
    x--;
    x |= x >> 1;  
    x |= x >> 2;  
    x |= x >> 4;  
    x |= x >> 8;  
    x |= x >> 16; 
    x++;
    return x;
}

constexpr int64_t roundUptoPowerOf2(int64_t x) noexcept {
    x--;
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    x |= x >> 32;
    x++;
    return x;
}

constexpr uint8_t roundUptoPowerOf2(uint8_t x) noexcept {
    x--;
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x++;
    return x;
}

constexpr uint16_t roundUptoPowerOf2(uint16_t x) noexcept {
    x--;
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x++;
    return x;
}

constexpr uint32_t roundUptoPowerOf2(uint32_t x) noexcept {
    x--;
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    x++;
    return x;
}

constexpr uint64_t roundUptoPowerOf2(uint64_t x) noexcept {
    x--;
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    x |= x >> 32;
    x++;
    return x;
}