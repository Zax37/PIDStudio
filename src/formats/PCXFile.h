#pragma once

#include "../File.h"

class PIDPalette;

class PCXFile : public File {
public:
    bool load(std::istream& stream) override;
    [[nodiscard]] const std::shared_ptr<PIDPalette>& getPalette() const;

private:
    std::shared_ptr<PIDPalette> palette = nullptr;
};
