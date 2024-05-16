#include "PIDPalette.h"

#ifdef DEBUG
#include <iostream>
#include <format>
#endif // DEBUG

bool PIDPalette::loadFromStream(std::ifstream& stream)
{
    try {
        stream >> data;
        return true;
    }
    catch (...) {
        return false;
    }
}

#ifdef DEBUG
void PIDPalette::printDebug() const
{
    int z = 0;
    for (int i = 0; i < 256; i++) {
        for (int j = 0; j < 3; j++) {
            std::cout << std::format("{:#04x}, ", (int)data[i][j]);
            if (++z == 12) {
                z = 0;
                std::cout << std::endl;
            }
        }
    }
}
#endif // DEBUG

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
        return sf::Color(data[i][0], data[i][1], data[i][2]);
    else return sf::Color::Transparent;
}
