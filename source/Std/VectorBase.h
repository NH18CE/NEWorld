// 
// NWShared: VecorBase.h
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

#include "Math.h"
#include <type_traits>
#include <utility>
#include <new>
#include "Config/Config.hpp"

NW_API void* vectorAlloc(size_t size, size_t item);
NW_API void* vectorRealloc(void* old, size_t size, size_t item);
NW_API void vectorFree(void* mem);

template <class T>
class Vector {
public:
    constexpr Vector() noexcept : mData(nullptr), mSize(0u), mCapcity(0u){}
    ~Vector() noexcept(std::is_nothrow_destructible_v<T>) { clear(); }
    void clear() noexcept(std::is_nothrow_destructible_v<T>) { for (auto& x : *this) x.~T(); mSize = 0; }
    auto size() const noexcept { return mSize; }
    auto empty() const noexcept { return !bool(size()); }
    auto capacity() const noexcept { return mCapcity; }
    void reserve(size_t cap) { if (cap > capacity()) doRealloc(cap); }
    void resize(size_t cap) {
        doResizeTrim(cap);
        if (cap > mSize)
            for (auto i = mSize; i < cap; ++i) new (mData + i)T();
    }
    void resize(size_t cap, const T& v) {
        doResizeTrim(cap);
        if (cap > mSize)
            for (auto i = mSize; i < cap; ++i) new (mData + i)T(v);
    }
    T& at(size_t p) noexcept { return mData[p]; }
    const T& at(size_t p) const noexcept { return mData[p]; }
    T& operator[](size_t p) noexcept { return at(p); }
    const T& operator[](size_t p) const noexcept { return at(p); }
    T& front() noexcept { return *mData; }
    const T& front() const noexcept { return *mData; }
    T& back() noexcept { return mData[mSize - 1]; }
    const T& back() const noexcept { return mData[mSize - 1]; }
    T* begin() noexcept { return mData; }
    const T* begin() const noexcept { return mData; }
    T* end() noexcept { return mData + mSize; }
    const T* end() const noexcept { return mData + mSize; }
    const T* cbegin() const noexcept { return mData; }
    const T* cend() const noexcept { return mData + mSize; }
    auto pushBack(const T& v) { return emplaceBack(v); }
    auto pushBack(T&& v) { return emplaceBack(std::move(v)); }
    template <class ...Ts>
    auto emplaceBack(Ts&&... args) {
        incSize();
        return new (&back()) T(std::forward<Ts>(args)...);
    }
    template <class ...Ts>
    auto emplace(const T* iter, Ts&&... args) {
        return new (doInsertion(iter - begin())) T(std::forward<Ts>(args)...);
    }
    auto insert(const T* iter, const T& v) { return emplace(iter, v); }
    auto insert(const T* iter, T&& v) { return emplace(iter, std::move(v)); }
    auto erase(T* iter) {
        iter->~T();
        for (auto i = iter + 1; i < end(); ++i)  *(i - 1) = std::move(*i);
    }
private:
    T* mData;
    size_t mSize, mCapcity;
    void incSize() { reserve(++mSize); }
    void doResizeTrim(const size_t cap) {
        reserve(cap);
        // Throw in the dtor will trash the vector! be carefull with that
        if (cap < mSize) {
            for (auto i = cap; i < mSize; ++i) mData[i].~T();
            mSize = cap;
        }
    }
    void doRealloc(size_t cap) {
        cap = roundUptoPowerOf2(cap);
        if constexpr(std::is_trivially_copyable_v<T>) {
            mData = reinterpret_cast<T*>(vectorRealloc(mData, cap, sizeof(T)));
        }
        else {
            auto newMem = reinterpret_cast<T*>(vectorAlloc(cap, sizeof(T)));
            if constexpr(std::is_nothrow_move_constructible_v<T>) {
                for (auto i = 0; i < mSize; ++i) newMem[i] = std::move(mData[i]);
            }
            else {
                for (auto i = 0; i < mSize; ++i) newMem[i] = mData[i];
            }
            std::swap(newMem, mData);
            vectorFree(newMem);
        }
        mCapcity = cap;
    }
    T* doInsertion(const size_t insPos, const size_t n = 1u) {
        if constexpr(std::is_trivially_copyable_v<T>) {
            if (mSize + n > mCapcity) {
                const auto cap = roundUptoPowerOf2(mSize + n);
                mData = reinterpret_cast<T*>(vectorRealloc(mData, cap, sizeof(T)));
                mCapcity = cap;
            }
            std::memmove(mData + insPos + n, mData + insPos, mSize - insPos);
        }
        else {
            if (mSize + n > mCapcity) {
                const auto cap = roundUptoPowerOf2(mSize + n);
                auto newMem = reinterpret_cast<T*>(vectorAlloc(cap, sizeof(T)));
                if constexpr(std::is_nothrow_move_constructible_v<T>) {
                    for (auto i = 0; i < insPos; ++i) newMem[i] = std::move(mData[i]);
                    for (auto i = insPos; i < mSize; ++i) newMem[i + n] = std::move(mData[i]);
                }
                else {
                    for (auto i = 0; i < insPos; ++i) newMem[i] = mData[i];
                    for (auto i = insPos; i < mSize; ++i) newMem[i + n] = mData[i];
                }
                std::swap(newMem, mData);
                vectorFree(newMem);
                mCapcity = cap;
            }
            else {
                for (int i = mSize - 1; i >= insPos; --i) mData[i + n] = std::move(mData[i]);
            }
        }
        mSize += n;
        return begin() + insPos;
    }
};
