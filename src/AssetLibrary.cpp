#include "PIDStudio.h"
#include "PIDPalette.h"
#include "PCXFile.h"
#include "SupportedGame.h"
#include "String.h"

#include <stack>

#include <imgui.h>

#include "imgui_internal.h"

#ifdef DEBUG
#include <iostream>
#endif // DEBUG

void palFileHandler(std::shared_ptr<AssetLibrary::TreeNode> node) {
    node->palette = std::make_shared<PIDPalette>();
    if (node->palette->loadFromFile(node->path)) {
        node->parent->palette = node->palette;
    }
}

void pcxFileHandler(std::shared_ptr<AssetLibrary::TreeNode> node) {
    std::shared_ptr<PCXFile> pcxFile = std::make_shared<PCXFile>();
    if (pcxFile->loadFromFile(node->path)) {
        node->palette = pcxFile->getPalette();
        node->parent->palette = node->palette;
    }
}

void pidFileHandler(std::shared_ptr<AssetLibrary::TreeNode> node) {
    node->isHidden = false;
}

std::unordered_map<std::string, AssetLibrary::FileHandler> AssetLibrary::supportedFileTypes = {
    {".pal", palFileHandler },
    {".pcx", pcxFileHandler },
    {".pid", pidFileHandler }
};

std::shared_ptr<AssetLibrary::TreeNode> AssetLibrary::TreeNode::resolve(const char* path)
{
    for (auto node : children) {
        if (stringEquals(node->name, path, false)) {
            return node;
        }
    }

    return nullptr;
}

std::shared_ptr<AssetLibrary::TreeNode> AssetLibrary::TreeNode::resolve(const char** paths)
{
    std::shared_ptr<AssetLibrary::TreeNode> node = resolve(*paths++);
    while (*paths && node) {
        node = node->resolve(*paths++);
    }

    return node;
}

AssetLibrary::AssetLibrary(PIDStudio* app, std::filesystem::path path, std::shared_ptr<SupportedGame> game) : app(app), path(path), game(game)
{
    requiresRebuilding = true;
    rebuildTreeIfRequired();

    efsw::WatchID watchID = fileWatcher.addWatch(path.string(), this, true);
    fileWatcher.watch();
}

void AssetLibrary::rebuildTreeIfRequired() {
    if (!requiresRebuilding) return;
#ifdef DEBUG
    std::cout << "Rebuilding asset tree" << std::endl;
#endif // DEBUG

    root = std::make_shared<AssetLibrary::TreeNode>();
    root->isHidden = false;

    populateTree(path, root);
    game->initializeLibrary(root);

    requiresRebuilding = false;
}

void AssetLibrary::populateTree(std::filesystem::path rootPath, std::shared_ptr<AssetLibrary::TreeNode> rootNode) {
    std::stack<std::pair<std::filesystem::path, std::shared_ptr<TreeNode>>> stack;
    stack.push({ rootPath, rootNode });

    while (!stack.empty()) {
        const auto [parentPath, parentNode] = stack.top();
        stack.pop();

        for (const auto& entry : std::filesystem::directory_iterator(parentPath)) {
            std::shared_ptr<AssetLibrary::TreeNode> childNode = std::make_shared<AssetLibrary::TreeNode>();
            childNode->parent = parentNode;
            childNode->path = entry.path();
            childNode->name = childNode->path.filename().string();
            parentNode->children.emplace_back(childNode);

            if (entry.is_directory()) {
                stack.push({ childNode->path, childNode });
            } else if (entry.is_regular_file()) {
                std::string ext = childNode->path.extension().string();
                std::transform(ext.begin(), ext.end(), ext.begin(), charToLower);

                if (!supportedFileTypes.contains(ext)) continue;

                supportedFileTypes[ext](childNode);

                if (!childNode->isHidden) {
                    std::shared_ptr<AssetLibrary::TreeNode> node = parentNode;
                    do {
                        node->isHidden = false;
                        node = node->parent;
                    } while (node && node->isHidden);
                }
            }
        }
    }
}

void AssetLibrary::displayContent() { displayContent(root); }

void AssetLibrary::displayContent(std::shared_ptr<AssetLibrary::TreeNode> node)
{
    if (node->isHidden) return;

    bool isLeaf = node->children.empty();
    ImGuiTreeNodeFlags flags = isLeaf ? ImGuiTreeNodeFlags_Leaf : ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;

    bool isOpen = ImGui::TreeNodeEx(node->name.c_str(), flags);

    if (ImGui::GetCurrentContext()->LastItemData.StatusFlags & ImGuiItemStatusFlags_HoveredRect || ImGui::IsPopupOpen(node->name.c_str())) {
        if (ImGui::BeginPopupContextItem(node->name.c_str())) {
            app->libraryEntryContextMenu(node, isLeaf);
            ImGui::EndPopup();
        }
    }

    if (isOpen) {
        if (isLeaf) {
            if (ImGui::GetCurrentContext()->LastItemData.StatusFlags & ImGuiItemStatusFlags_ToggledSelection) {
                app->openLibraryFile(node);
            }
        } else {
            for (const auto& child : node->children) {
                displayContent(child);
            }
        }
        ImGui::TreePop();
    }
}

inline void AssetLibrary::handleFileAction(efsw::WatchID watchid, const std::string& dir, const std::string& filename, efsw::Action action, std::string oldFilename) {
    
    // TODO: could simply add/remove nodes instead of rebuilding entire tree, but I'll start with that as simpler approach
    requiresRebuilding = true;

    /*
    std::filesystem::path relative = std::filesystem::proximate(dir, path);
    std::shared_ptr<AssetLibrary::TreeNode> node = root;

    for (auto& segment : relative) {
        std::string segmentString = segment.string();
        std::shared_ptr<AssetLibrary::TreeNode> childNode = node->resolve(segmentString.c_str());
        
        if (!childNode) {
            node->children.
        }
    }
    switch (action) {
        case efsw::Actions::Add:
        std::cout << "DIR (" << relative << ") FILE (" << filename << ") has event Added"
        << std::endl;
        break;
        case efsw::Actions::Delete:
        std::cout << "DIR (" << relative << ") FILE (" << filename << ") has event Delete"
        << std::endl;
        break;
        case efsw::Actions::Modified:
        std::cout << "DIR (" << relative << ") FILE (" << filename << ") has event Modified"
        << std::endl;
        break;
        case efsw::Actions::Moved:
        std::cout << "DIR (" << relative << ") FILE (" << filename << ") has event Moved from ("
        << oldFilename << ")" << std::endl;
        break;
        default:
        std::cout << "Should never happen!" << std::endl;
    }
    */
}
