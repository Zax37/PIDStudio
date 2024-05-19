#pragma once

#include "File.h"

class PIDPalette;

class PCXFile : public File {
public:
    bool loadFromStream(std::ifstream& stream) override;
    [[nodiscard]] const std::shared_ptr<PIDPalette>& getPalette() const;

private:
    std::shared_ptr<PIDPalette> palette = nullptr;
};
