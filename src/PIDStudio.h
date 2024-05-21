#pragma once

#include "AssetLibrary.h"

#include <imgui.h>
#include <SFML/Graphics/RenderWindow.hpp>

#define MINI_CASE_SENSITIVE
#include <mini/ini.h>

#include <set>

class PIDFile;
class PIDPalette;
class SupportedGame;

class PIDStudio {
	enum OPENED_FILE_WINDOW_RESULT {
		NONE,
		CLOSE,
		KEEP_OPEN,
	};
public:
	PIDStudio();
	~PIDStudio();

	int run();
	void openLibraryFile(const std::shared_ptr<AssetLibrary>& library, const std::shared_ptr<AssetLibrary::TreeNode>& node, bool inSeparateWindow = false);
	void libraryEntryContextMenu(const std::shared_ptr<AssetLibrary>& library, const std::shared_ptr<AssetLibrary::TreeNode>& node, bool isLeaf, bool isRoot);
	std::shared_ptr<PIDPalette> getDefaultPalette() { return defaultPalette; }

private:
	std::vector<std::shared_ptr<SupportedGame>> supportedGames;
	std::shared_ptr<SupportedGame> claw;

	mINI::INIStructure settings;
	sf::RenderWindow mainWindow;
	std::vector<std::shared_ptr<AssetLibrary>> assetLibraries;

	std::vector<std::shared_ptr<PIDFile>> openedFiles;
	std::shared_ptr<PIDFile> openedLibraryFile;
	std::shared_ptr<PIDFile> currentlyFocusedFile;
	std::set<std::shared_ptr<PIDFile>> filesToClose;
    std::shared_ptr<AssetLibrary> libraryToClose;

	std::shared_ptr<PIDPalette> currentPalette;
	std::shared_ptr<PIDPalette> defaultPalette;

	PIDFile* bringFocusTo = nullptr;

	ImGuiID dockspaceId{},
			dockspaceIdLeft{},
			dockspaceIdRight{},
			dockspaceIdRightTop{},
			dockspaceIdRightBottom{};

	void menuBar();
	void toolBar();
	void preDockedWindows();
	void paletteWindow();
	void metadataWindow();
	void libraryWindow();

	void closeContextMenu();
	void saveAsContextMenu();

	void openedFilesWindows();
	OPENED_FILE_WINDOW_RESULT openedFileWindow(const std::shared_ptr<PIDFile>& file);

	void keepLibraryFileOpened();
	void closeFile(const std::shared_ptr<PIDFile>& file);
	void closeAllFiles();

	void openPidFileDialog();
	void addLibraryDialog();
	void addLibrary(std::filesystem::path& path, const std::shared_ptr<SupportedGame>& game);
	bool isFileAlreadyOpen(const std::filesystem::path& path, PIDFile** outFilePtr = nullptr);
	void openAllFiles(const std::shared_ptr<AssetLibrary>& library, const std::shared_ptr<AssetLibrary::TreeNode>& node);
	void loadPaletteFromFile();
	void savePaletteToFile();
};
