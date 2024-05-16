#pragma once

#include "File.h"

class PIDPalette;

class PCXFile : public File {
public:
    bool loadFromStream(std::ifstream& stream) override;
    std::shared_ptr<PIDPalette> getPalette() const { return palette; }

private:
    std::shared_ptr<PIDPalette> palette = nullptr;
};
