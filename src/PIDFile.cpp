#include "PIDFile.h"

#include "PIDPalette.h"
#include "PIDStudio.h"

#ifdef DEBUG
#include <iostream>
#endif // DEBUG

bool PIDFile::loadFromFile(const std::filesystem::path& filepath) {
    this->path = filepath;
    name = filepath.filename().string();
    windowName = name + "###" + filepath.string();

    return File::loadFromFile(filepath);
}

bool PIDFile::loadFromStream(std::ifstream& stream) {
    stream >> magic >> flags >> width >> height >> offsetX >> offsetY >> unknown;

    #ifdef DEBUG
    if (magic != 10) {
        std::cout << "Unexpected magic number" << std::endl;
    }
    #endif // DEBUG

    if (flags & Flag_OwnPalette) {
        stream.seekg(-768, std::ios_base::end);
        palette = std::make_shared<PIDPalette>();
        palette->loadFromStream(stream);
        stream.seekg(32);
    }

    data = new uint8_t[width * height];
    uint8_t* outPtr = data;
    uint8_t* endPtr = outPtr + width * height;

    int length;
    uint8_t currentByte;

    auto outputCurrentByte = [&](){ *outPtr++ = currentByte; };
    auto fillWithCurrentByte = [&]() { memset(outPtr, currentByte, length); outPtr += length; };
    auto fillWithZeros = [&]() { currentByte = 0; fillWithCurrentByte(); };
    auto readAndOutputBytes = [&]() {
        for (int i = 0; i < length; i++) {
            stream >> currentByte;
            outputCurrentByte();
        }
    };

    auto readCompressedPixels = [&]() {
        while (outPtr < endPtr) {
            stream >> currentByte;
            if (currentByte > 128) {
                length = currentByte - 128;
                fillWithZeros();
            }
            else {
                length = currentByte;
                readAndOutputBytes();
            }
        }
    };

    auto readUncompressedPixels = [&]() {
        while (outPtr < endPtr) {
            stream >> currentByte;
            if (currentByte > 192) {
                length = currentByte - 192;
                stream >> currentByte;
                fillWithCurrentByte();
            }
            else {
                outputCurrentByte();
            }
        }
    };

    if (flags & Flag_Compression) {
        readCompressedPixels();
    } else {
        readUncompressedPixels();
    }

    return true;
}

const sf::Texture& PIDFile::getTexture()
{
    if (requiresTextureUpdate) {
        const std::shared_ptr<PIDPalette>& imagePalette = palette ? palette : app->getDefaultPalette();

        sf::Image img;
        img.create(width, height);

        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++)
            {
                img.setPixel(x, y, imagePalette->getColor(data[y * width + x]));
            }
        }

        texture.loadFromImage(img);
        texture.setSmooth(true);

        requiresTextureUpdate = false;
    }
    return texture;
}

std::string PIDFile::getFlagsDescription()
{
    std::string flagsDescription;

    if (flags & Flag_Transparency) flagsDescription += "Flag_Transparency\n";
    if (flags & Flag_VideoMemory) flagsDescription += "Flag_VideoMemory\n";
    if (flags & Flag_SystemMemory) flagsDescription += "Flag_SystemMemory\n";
    if (flags & Flag_Mirror) flagsDescription += "Flag_Mirror\n";
    if (flags & Flag_Invert) flagsDescription += "Flag_Invert\n";
    if (flags & Flag_Compression) flagsDescription += "Flag_Compression\n";
    if (flags & Flag_Lights) flagsDescription += "Flag_Lights\n";
    if (flags & Flag_OwnPalette) flagsDescription += "Flag_OwnPalette\n";

    return flagsDescription;
}
