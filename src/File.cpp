#include "File.h"

bool File::loadFromFile(std::filesystem::path path)
{
    std::ifstream stream(path, std::ios_base::binary | std::ios_base::in);
    if (stream.good()) {
        return loadFromStream(stream);
    }

    return false;
}
