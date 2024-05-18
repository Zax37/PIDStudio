#include "SupportedGame.h"

#include <libintl.h>

int getNumSuffix(const std::string& s) {
    size_t pos = s.find_last_not_of("0123456789");
    return (pos != std::string::npos && pos + 1 < s.length()) ? std::stoi(s.substr(pos + 1)) : 0;
};

bool sortElements(std::shared_ptr<AssetLibrary::TreeNode>& node1, std::shared_ptr<AssetLibrary::TreeNode>& node2)
{
    int numSuffix1 = getNumSuffix(node1->name);
    int numSuffix2 = getNumSuffix(node2->name);

    // Compare based on numerical suffix if both strings have one, otherwise compare strings directly
    return (numSuffix1 != 0 && numSuffix2 != 0) ? numSuffix1 < numSuffix2 : node1->name < node2->name;
}

void SupportedGame::initializeLibrary(std::shared_ptr<AssetLibrary::TreeNode> root) {
    root->name = gettext(name);
    std::sort(root->children.begin(), root->children.end(), sortElements);
}

std::string SupportedGame::getNames(std::vector<std::shared_ptr<SupportedGame>> games) {
    switch (games.size()) {
    case 0:
        return "";
    case 1:
        return games[0]->name;
    }

    std::vector<std::shared_ptr<SupportedGame>>::iterator it = games.begin();
    std::string names = (*it)->name;

    while (++it != games.end()) {
        names += ", ";
        names += (*it)->name;
    }

    return names;
}
