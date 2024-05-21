#pragma once

#include <filesystem>
#include <fstream>

template<typename T>
std::istream& operator>(std::istream& ifs, T& data) {
    ifs.read(reinterpret_cast<char*>(&data), sizeof(data));
    return ifs;
}

template<typename T>
std::ostream& operator<(std::ostream& ofs, const T& data) {
    ofs.write(reinterpret_cast<const char*>(&data), sizeof(data));
    return ofs;
}

class File {
public:
    virtual bool loadFromFile(const std::filesystem::path& path);
    virtual bool loadFromStream(std::istream& stream);
    virtual bool saveToFile(const std::filesystem::path& path);
    virtual bool saveToStream(std::ostream& stream);
protected:
    virtual bool load(std::istream& stream) { return false; }
    virtual bool save(std::ostream& stream) { return false; }
};
