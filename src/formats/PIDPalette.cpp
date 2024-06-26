#include "PIDPalette.h"

#include <algorithm>

#ifdef DEBUG
#include <iostream>
#include <fmt/core.h>

void PIDPalette::printDebug() const
{
    int z = 0;
    for (int i = 0; i < 256; i++) {
        for (int j = 0; j < 3; j++) {
            std::cout << fmt::format("{:#04x}, ", (int)data[i][j]);
            if (++z == 12) {
                z = 0;
                std::cout << std::endl;
            }
        }
    }
}
#endif // DEBUG

PIDPalette::PIDPalette(const unsigned char* ptr)
{
    std::copy(ptr, ptr+sizeof(data), (unsigned char*)data);
}

bool PIDPalette::load(std::istream& stream)
{
    stream > data;
    return true;
}

bool PIDPalette::save(std::ostream &stream) {
    stream < data;
    return true;
}

const sf::Texture& PIDPalette::getTexture()
{
    if (requiresTextureUpdate) {
        sf::Image img;
        img.create(97, 97);
        for (int y = 0; y < 16; y++) {
            for (int x = 0; x < 16; x++)
            {
                for (int i = 0; i < 5; i++)
                    for (int j = 0; j < 5; j++)
                        img.setPixel(x * 6 + i + 1, y * 6 + j + 1, getColor(y * 16 + x, false));
            }
        }
        texture.loadFromImage(img);
        texture.setSmooth(true);

        requiresTextureUpdate = false;
    }
    return texture;
}

sf::Color PIDPalette::getColor(int i, bool treatFirstAsTransparent) const
{
    if (i || !treatFirstAsTransparent)
        return {data[i][0], data[i][1], data[i][2]};
    else return sf::Color::Transparent;
}
