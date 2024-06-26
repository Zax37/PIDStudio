#include "PCXFile.h"
#include "PIDPalette.h"

#define SUPPORTED_VERSION 0x05

bool PCXFile::load(std::istream& stream) {
    uint8_t version;
    stream.seekg(1);
    stream > version;

    if (version != SUPPORTED_VERSION) {
        return false;
    }

    stream.seekg(-768, std::ios_base::end);
    palette = std::make_shared<PIDPalette>();
    palette->loadFromStream(stream);

    return true;
}

const std::shared_ptr<PIDPalette>& PCXFile::getPalette() const {
    return palette;
}
