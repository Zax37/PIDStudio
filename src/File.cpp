#include "File.h"

bool File::loadFromFile(const std::filesystem::path& path)
{
    try {
        std::ifstream stream(path, std::ios_base::binary | std::ios_base::in);
        return loadFromStream(stream);
    } catch (...) {
        return false;
    }
}

bool File::saveToFile(const std::filesystem::path &path) {
    try {
        std::ofstream stream(path, std::ios_base::binary | std::ios_base::out);
        return saveToStream(stream);
    } catch (...) {
        return false;
    }
}

bool File::loadFromStream(std::istream &stream) {
    try {
        return stream.good() && load(stream);
    } catch (...) {
        return false;
    }
}

bool File::saveToStream(std::ostream &stream) {
    try {
        return stream.good() && save(stream);
    } catch (...) {
        return false;
    }
}
