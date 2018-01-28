// 
// NEWorld: ThreadWorker.cpp
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

#include "ThreadWorker.h"

namespace World::Threading {
    Pool::Pool() : mReaderStep(std::thread::hardware_concurrency()), mDetachedStep(std::thread::hardware_concurrency() - 1) {
        for (auto i = 0u; i < std::thread::hardware_concurrency(); ++i)
            mWorkers.emplace_back([this, i]() { work(i); });
    }

    void Pool::processRead() {    
    }

    void Pool::processWrite() {
        mReaders.store(mWorkers.size());
    }

    void Pool::processDetached() {
        while (!mReaders) {
            
        }
    }

    void Pool::work(const int id) {
        while(mWorking) {
            processRead();
            if (mReaders.fetch_sub(1) == 1)
                processWrite();
            else
                processDetached();
        }
    }
}
