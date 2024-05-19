#include "Claw.h"

#include "../String.h"
#include "../PIDStudio.h"
#include "../PIDPalette.h"

#include "../assets/claw_game.pal.h"

void initializeStatesPalettes(const std::shared_ptr<AssetLibrary::TreeNode>& statesDirectory) {
    static const char* attractScreensName[] = { "ATTRACT", "SCREENS", nullptr };
    auto attractScreensDirectory = statesDirectory->resolve(attractScreensName);

    if (!attractScreensDirectory) return;

    statesDirectory->palette = attractScreensDirectory->palette;

    auto bootyDirectory = statesDirectory->resolve("BOOTY");
    if (!bootyDirectory) return;

    static const char* imagesMappieceName[] = { "IMAGES", "MAPPIECE", nullptr };
    auto imagesMappieceDirectory = bootyDirectory->resolve(imagesMappieceName);

    if (!imagesMappieceDirectory || imagesMappieceDirectory->children.size() < 14) return;

    auto screensDirectory = bootyDirectory->resolve("SCREENS");

    if (!screensDirectory || screensDirectory->children.size() < 29) return;

    for (int i = 0; i < 14; i++) {
        std::stringstream stream;
        stream << "LEVEL" << std::setfill('0') << std::setw(3) << (i + 1);
        std::string levelNameStr = stream.str();

        auto levelDirectory = imagesMappieceDirectory->resolve(levelNameStr.c_str());

        if (!levelDirectory) continue;

        auto child = screensDirectory->children[i * 2];
        levelDirectory->palette = child->palette;
    }

    bootyDirectory->palette = screensDirectory->children[0]->palette;
}

void Claw::initializeLibrary(const std::shared_ptr<AssetLibrary::TreeNode>& root) {
    SupportedGame::initializeLibrary(root);

    for (const auto& node : root->children) {
        switch (charToLower(node->name[0])) {
        case 'g': // GAME
        {
            static const char* imagesLightfxName[] = { "IMAGES", "LIGHTFX", 0 };
            const auto& imagesLightfxDir = node->resolve(imagesLightfxName);
            if (imagesLightfxDir) {
                imagesLightfxDir->palette = app->getDefaultPalette();
            }
        } // fall-through
        case 'c': // CLAW
            node->palette = std::make_shared<PIDPalette>(CLAW_GAME_PAL);
            break;
        case 'l': // LEVELX
        {
            const auto& paletteDir = node->resolve("PALETTES");
            if (paletteDir) {
                node->palette = paletteDir->palette;
            }
            break;
        }
        case 's': // STATES
            initializeStatesPalettes(node);
            break;
        }
    }
}
