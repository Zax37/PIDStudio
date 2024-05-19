#include "AssetLibrary.h"

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

void palFileHandler(const std::shared_ptr<AssetLibrary::TreeNode> &node) {
    node->palette = std::make_shared<PIDPalette>();
    if (node->palette->loadFromFile(node->path)) {
        node->parent->palette = node->palette;
    }
}

void pcxFileHandler(const std::shared_ptr<AssetLibrary::TreeNode> &node) {
    std::shared_ptr<PCXFile> pcxFile = std::make_shared<PCXFile>();
    if (pcxFile->loadFromFile(node->path)) {
        node->palette = pcxFile->getPalette();
        node->parent->palette = node->palette;
    }
}

void pidFileHandler(const std::shared_ptr<AssetLibrary::TreeNode> &node) {
    node->isHidden = false;
}

std::unordered_map<std::string, AssetLibrary::FileHandler> AssetLibrary::supportedFileTypes = {
        {".pal", palFileHandler},
        {".pcx", pcxFileHandler},
        {".pid", pidFileHandler}
};

std::shared_ptr<AssetLibrary::TreeNode> AssetLibrary::TreeNode::resolve(const char *resolvePath) {
    for (auto node: children) {
        if (stringEquals(node->name, resolvePath, false)) {
            return node;
        }
    }

    return nullptr;
}

std::shared_ptr<AssetLibrary::TreeNode> AssetLibrary::TreeNode::resolve(const char **resolvePaths) {
    std::shared_ptr<AssetLibrary::TreeNode> node = resolve(*resolvePaths++);
    while (*resolvePaths && node) {
        node = node->resolve(*resolvePaths++);
    }

    return node;
}

AssetLibrary::AssetLibrary(PIDStudio *app, const std::filesystem::path &path,
                           const std::shared_ptr<SupportedGame> &game) : app(app), path(path), game(game) {
    requiresRebuilding = true;
    rebuildTreeIfRequired();

    fileWatcher.addWatch(path.string(), this, true);
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

void AssetLibrary::populateTree(const std::filesystem::path &rootPath,
                                const std::shared_ptr<AssetLibrary::TreeNode> &rootNode) {
    std::stack<std::pair<std::filesystem::path, std::shared_ptr<TreeNode>>> stack;
    stack.emplace(rootPath, rootNode);

    while (!stack.empty()) {
        const auto [parentPath, parentNode] = stack.top();
        stack.pop();

        for (const auto &entry: std::filesystem::directory_iterator(parentPath)) {
            std::shared_ptr<AssetLibrary::TreeNode> childNode = std::make_shared<AssetLibrary::TreeNode>();
            childNode->parent = parentNode;
            childNode->path = entry.path();
            childNode->name = childNode->path.filename().string();
            parentNode->children.emplace_back(childNode);

            if (entry.is_directory()) {
                stack.emplace(childNode->path, childNode);
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

void AssetLibrary::displayContent() {
    std::stack<std::shared_ptr<TreeNode>> stack;
    stack.emplace(root);

    while (!stack.empty()) {
        const auto node = stack.top();
        stack.pop();

        if (!node) {
            ImGui::TreePop();
            continue;
        }
        if (node->isHidden) { continue; }

        bool isLeaf = node->children.empty();
        ImGuiTreeNodeFlags flags = isLeaf ? ImGuiTreeNodeFlags_Leaf
                                          : ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;

        bool isOpen = ImGui::TreeNodeEx(node->name.c_str(), flags);

        if (ImGui::GetCurrentContext()->LastItemData.StatusFlags & ImGuiItemStatusFlags_HoveredRect
            || ImGui::IsPopupOpen(node->name.c_str())) {
            if (ImGui::BeginPopupContextItem(node->name.c_str())) {
                app->libraryEntryContextMenu(shared_from_this(), node, isLeaf, node == root);
                ImGui::EndPopup();
            }
        }

        if (isOpen) {
            if (isLeaf) {
                if (ImGui::GetCurrentContext()->LastItemData.StatusFlags & ImGuiItemStatusFlags_ToggledSelection) {
                    app->openLibraryFile(shared_from_this(), node);
                }
                ImGui::TreePop();
            } else {
                stack.emplace(nullptr); // delay ImGui::TreePop() to remain correct structure
                for (auto it = node->children.rbegin(); it != node->children.rend(); ++it) {
                    stack.emplace(*it);
                }
            }
        }
    }
}

inline void AssetLibrary::handleFileAction(efsw::WatchID watchid, const std::string &dir, const std::string &filename,
                                           efsw::Action action, std::string oldFilename) {

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

bool AssetLibrary::hasFilepath(const std::filesystem::path &filepath,
                               std::shared_ptr<AssetLibrary::TreeNode> &outFoundNode) {
    auto mismatch = std::mismatch(this->path.begin(), this->path.end(), filepath.begin());
    bool isFilepathWithinLibrary = mismatch.first == this->path.end();

    if (isFilepathWithinLibrary) {
        outFoundNode = root;
        for (auto it = mismatch.second; it != filepath.end(); it++) {
            outFoundNode = outFoundNode->resolve(it->string().c_str());
            if (!outFoundNode) return false;
        }
    }

    return outFoundNode != nullptr;
}

std::shared_ptr<PIDPalette> AssetLibrary::inferPalette(const std::shared_ptr<AssetLibrary::TreeNode> &node) {
    std::shared_ptr<PIDPalette> palette = node->palette;
    std::shared_ptr<AssetLibrary::TreeNode> parent = node->parent;

    while (parent && !palette) {
        palette = parent->palette;
        parent = parent->parent;
    }

    return palette;
}

const char *AssetLibrary::getIniKey() const {
    return game->getIniKey();
}
