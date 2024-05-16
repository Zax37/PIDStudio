#include "PCXFile.h"
#include "PIDPalette.h"

#define SUPPORTED_VERSION 0x05

bool PCXFile::loadFromStream(std::ifstream& stream) {
    unsigned char version;
    stream.seekg(1);
    stream >> version;
    
    if (version != SUPPORTED_VERSION) {
        return false;
    }

    stream.seekg(-768, std::ios_base::end);
    palette = std::make_shared<PIDPalette>();
    palette->loadFromStream(stream);
}
