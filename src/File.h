#pragma once

#include <filesystem>
#include <fstream>

template<typename T>
std::ifstream& operator>>(std::ifstream& ifs, T& data) {
    ifs.read(reinterpret_cast<char*>(&data), sizeof(data));
    return ifs;
}

template<typename T>
std::ofstream& operator<<(std::ofstream& ofs, const T& data) {
    ofs.write(reinterpret_cast<const char*>(&data), sizeof(data));
    return ofs;
}

class File {
public:
    virtual bool loadFromFile(const std::filesystem::path& path);
    virtual bool loadFromStream(std::ifstream& stream) { return false; };
};
