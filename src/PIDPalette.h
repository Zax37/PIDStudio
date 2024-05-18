#pragma once

#include "File.h"

#include <SFML/Graphics/Texture.hpp>

class PIDPalette : public File
{
public:
    PIDPalette() {};
    PIDPalette(const unsigned char* ptr);

    sf::Color getColor(int i, bool treatFirstAsTransparent = true) const;
    // unsigned char FindColor(sf::Color color);

    bool loadFromStream(std::ifstream& stream) override;

    const sf::Texture& getTexture();

#ifdef DEBUG
    void printDebug() const;
#endif // DEBUG

private:
    unsigned char data[256][3];
    sf::Texture texture;
    bool requiresTextureUpdate = true;
};
