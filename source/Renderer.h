// 
// NEWorld: Renderer.h
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
#include "Shader.h"

namespace Renderer {
    //我猜你肯定不敢看Renderer.cpp  --qiaozhanrong
    //猜对了  --Null

    enum {
        MainShader,
        MergeFaceShader,
        ShadowShader,
        DepthShader
    };

    extern bool AdvancedRender;
    extern int ShadowRes;
    extern int maxShadowDist;
    extern int shadowdist;
    extern float sunlightXrot, sunlightYrot;
    extern unsigned int DepthTexture;
    extern std::vector<Shader> shaders;
    extern int ActiveShader;

    void initShaders();

    inline void bindShader(int shaderID) {
        shaders[shaderID].bind();
        ActiveShader = shaderID;
    }

    void destroyShaders();
    void EnableShaders();
    void DisableShaders();
    void StartShadowPass();
    void EndShadowPass();

    using VertexBufferID = GLuint;

    class VertexFormat {
    public:
        // Vertex attribute count
        size_t textureCount = 0, colorCount = 0, normalCount = 0, coordinateCount = 0, otherAttrCount = 0;
        // Vertex attributes count (sum of all)
        int vertexAttributeCount = 0;

        constexpr VertexFormat() = default;

        constexpr VertexFormat(int textureElementCount, int colorElementCount, int normalElementCount,
                               int coordinateElementCount, int oAttrC = 0)
            : textureCount(textureElementCount), colorCount(colorElementCount), normalCount(normalElementCount),
              coordinateCount(coordinateElementCount),
              vertexAttributeCount(
                  textureElementCount + colorElementCount + normalElementCount + coordinateElementCount + oAttrC) { }
    };

    class VertexArray {
    public:
        VertexArray(int maxVertexes, const VertexFormat& format) :
            mMaxVertexes(maxVertexes), mVertexes(0), mFormat(format),
            mData(std::make_unique<float[]>(mMaxVertexes * format.vertexAttributeCount)) {}

        VertexArray(const VertexArray&) = delete;
        VertexArray& operator=(const VertexArray&) = delete;
        VertexArray(VertexArray&&) = delete;
        VertexArray& operator=(VertexArray&&) = delete;
        ~VertexArray() = default;

        void clear() noexcept { mVertexes = 0; }

        void addPrimitive(size_t size, std::initializer_list<float> d) noexcept {
            std::copy(d.begin(), d.end(), mData.get() + mVertexes * mFormat.vertexAttributeCount);
            mVertexes += size;
        }

        // Get current vertex format
        const VertexFormat& getFormat() const noexcept { return mFormat; }

        // Get current vertex data
        const float* getData() const noexcept { return mData.get(); }

        // Get current vertex count
        size_t getVertexCount() const noexcept { return mVertexes; }

        // Set current vertex format
        void setFormat(const VertexFormat& format) {
            if (mFormat.vertexAttributeCount < format.vertexAttributeCount)
                mData = std::make_unique<float[]>(mMaxVertexes * format.vertexAttributeCount);
            mVertexes = 0;
            mFormat = format;
        }

    private:
        // Max vertex count
        const size_t mMaxVertexes;
        // Vertex count
        size_t mVertexes;
        // Vertex array format
        VertexFormat mFormat;
        // Vertex array
        std::unique_ptr<float[]> mData;
    };

    class VertexBuffer {
    public:
        VertexBuffer() : id(0), vertexes(0) {}

        VertexBuffer(VertexBuffer&& rhs) noexcept :
            id(rhs.id), vertexes(rhs.vertexes), format(rhs.format) {
            rhs.vertexes = rhs.id = 0;
            rhs.format = VertexFormat();
        }

        VertexBuffer(const VertexBuffer&) = delete;
        VertexBuffer& operator=(const VertexBuffer&) = delete;

        explicit VertexBuffer(const VertexArray& va);

        VertexBuffer& operator=(VertexBuffer&& rhs) noexcept {
            id = rhs.id;
            vertexes = rhs.vertexes;
            format = rhs.format;
            rhs.vertexes = rhs.id = 0;
            rhs.format = VertexFormat();
            return *this;
        }

        ~VertexBuffer() { destroy(); }

        // upload new data
        void update(const VertexArray& va);

        // Render vertex buffer
        void render() const;

        // Destroy vertex buffer
        void destroy();

        bool isEmpty() const noexcept { return vertexes == 0; }

    private:
        // Buffer ID
        VertexBufferID id;
        // Vertex count
        int vertexes;
        // Buffer format
        VertexFormat format;
    };

}
