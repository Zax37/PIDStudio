#pragma once

#include "../File.h"

#include "SFML/Graphics/Texture.hpp"

#include "imgui.h"
#include "imgui-SFML.h"

class PIDStudio;
class PIDPalette;

class PIDFile : public File {
    enum FLAGS {
        Flag_Transparency = 1,
        Flag_VideoMemory = 2,
        Flag_SystemMemory = 4,
        Flag_Mirror = 8,
        Flag_Invert = 16,
        Flag_Compression = 32,
        Flag_Lights = 64,
        Flag_OwnPalette = 128
    };

public:
    explicit PIDFile(PIDStudio* app) : app(app) {};
    ~PIDFile() { delete[] data; }
    bool loadFromFile(const std::filesystem::path& filepath) override;
    bool load(std::istream& stream) override;

    const std::string& getName() const { return name; }
    const std::string& getWindowName() const { return windowName; }
    const std::filesystem::path& getPath() const { return path; }
    const sf::Texture& getTexture();
    int getWidth() const { return width; }
    int getHeight() const { return height; }
    std::shared_ptr<PIDPalette> getPalette() const { return palette; }
    void setPalette(const std::shared_ptr<PIDPalette>& p) { palette = p; requiresTextureUpdate = true; }
    int getOffsetX() const { return offsetX; };
    int getOffsetY() const { return offsetY; };
    int* getUserData() { return unknown; };
    FLAGS getFlags() { return flags; };
    std::string getFlagsDescription();

private:
    PIDStudio* app;
    bool isModified = false;
    int width, height, magic, offsetX, offsetY, unknown[2];
    FLAGS flags;
    uint8_t* data = nullptr;
    std::shared_ptr<PIDPalette> palette;
    std::string name;
    std::string windowName;
    std::filesystem::path path;
    sf::Texture texture;
    bool requiresTextureUpdate = true;
};
