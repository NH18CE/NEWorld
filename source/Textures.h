// 
// NEWorld: Textures.h
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
#include "Definitions.h"

extern int BLOCKTEXTURE_SIZE, BLOCKTEXTURE_UNITSIZE, filter;
const short BITMAP_ID = 0x4D42;

namespace Textures {
#pragma pack(push)
#pragma pack(1)
    struct TEXTURE_RGB {
        unsigned int sizeX;
        unsigned int sizeY;
        std::unique_ptr<uint8_t[]> buffer;
    };

    struct TEXTURE_RGBA {
        unsigned int sizeX;
        unsigned int sizeY;
        std::unique_ptr<uint8_t[]> buffer;
    };

    struct BITMAPINFOHEADER {
        int biSize = 40, biWidth, biHeight;
        short biPlanes = 1, biBitCount = 24;
        int biCompression = 0, biSizeImage, biXPelsPerMeter = 0, biYPelsPerMeter = 0, biClrUsed = 0, biClrImportant = 0;
    };

    struct BITMAPFILEHEADER {
        short bfType = BITMAP_ID;
        int bfSize;
        short bfReserved1 = 0, bfReserved2 = 0;
        int bfOffBits = 54;
    };
#pragma pack(pop)

    enum BlockTextureID {
        ROCK,
        GRASS_TOP,
        GRASS_SIDE,
        DIRT,
        STONE,
        PLANK,
        WOOD_TOP,
        WOOD_SIDE,
        BEDROCK,
        LEAF,
        GLASS,
        WATER,
        LAVA,
        GLOWSTONE,
        SAND,
        CEMENT,
        ICE,
        COAL,
        IRON,
        TNT,
        UNKNOWN
    };

    const int NULLBLOCK = 63;

    void Init();
    uint8_t getTextureIndex(Block blockname, uint8_t side);
    float getTexcoordX(item item, uint8_t side);
    float getTexcoordY(item item, uint8_t side);
    void LoadRGBImage(TEXTURE_RGB& tex, const std::string& Filename);
    void LoadRGBAImage(TEXTURE_RGBA& tex, const std::string& Filename, const std::string& MkFilename);

    TextureID LoadRGBTexture(const std::string& Filename);
    TextureID LoadFontTexture(const std::string& Filename);
    TextureID LoadRGBATexture(const std::string& Filename, const std::string& MkFilename);
    TextureID LoadBlock3DTexture(const std::string& Filename, const std::string& MkFilename);

    void SaveRGBImage(const std::string& filename, TEXTURE_RGB& image);

    void Build2DMipmaps(GLenum format, int w, int h, int level, const uint8_t* src);
}
