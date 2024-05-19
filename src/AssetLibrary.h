#pragma once

#include <efsw/efsw.hpp>
#include <filesystem>
#include <vector>
#include <unordered_map>

class PIDPalette;
class SupportedGame;

class AssetLibrary : public efsw::FileWatchListener, public std::enable_shared_from_this<AssetLibrary>
{
public:
    struct TreeNode {
        std::shared_ptr<TreeNode> parent;
        std::string name;
        std::vector<std::shared_ptr<TreeNode>> children;
        std::shared_ptr<PIDPalette> palette;
        std::filesystem::path path;
        bool isHidden = true;

        std::shared_ptr<TreeNode> resolve(const char* path);
        std::shared_ptr<TreeNode> resolve(const char** path);
    };
    typedef void (*FileHandler)(const std::shared_ptr<AssetLibrary::TreeNode>&);

    AssetLibrary(class PIDStudio* app, const std::filesystem::path& path, const std::shared_ptr<SupportedGame>& game);

    void displayContent();
    void handleFileAction(efsw::WatchID watchid, const std::string& dir, const std::string& filename, efsw::Action action, std::string oldFilename) override;

    void rebuildTreeIfRequired();
    bool hasFilepath(const std::filesystem::path& filepath, std::shared_ptr<AssetLibrary::TreeNode>& outFoundNode);
    static std::shared_ptr<PIDPalette> inferPalette(const std::shared_ptr<AssetLibrary::TreeNode>& node) ;
private:
    static std::unordered_map<std::string, FileHandler> supportedFileTypes;

    class PIDStudio* app;
    efsw::FileWatcher fileWatcher;
    std::filesystem::path path;
    std::shared_ptr<TreeNode> root;
    std::shared_ptr<SupportedGame> game;
    bool requiresRebuilding;

    static void populateTree(const std::filesystem::path& path, const std::shared_ptr<AssetLibrary::TreeNode>& node);
};
