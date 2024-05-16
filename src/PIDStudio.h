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
	void openLibraryFile(std::shared_ptr<AssetLibrary::TreeNode> node, bool inSeparateWindow = false);
	void libraryEntryContextMenu(std::shared_ptr<AssetLibrary::TreeNode> node, bool isLeaf);
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

	std::shared_ptr<PIDPalette> currentPalette;
	std::shared_ptr<PIDPalette> defaultPalette;

	PIDFile* bringFocusTo;

	ImGuiID dockspaceId,
			dockspaceIdLeft,
			dockspaceIdRight,
			dockspaceIdRightTop,
			dockspaceIdRightBottom;

	void menuBar();
	void toolBar();
	void preDockedWindows();
	void paletteWindow();
	void metadataWindow();
	void libraryWindow();

	void closeContextMenu();
	void saveAsContextMenu();

	void openedFilesWindows();
	OPENED_FILE_WINDOW_RESULT openedFileWindow(std::shared_ptr<PIDFile> file);

	void closeFile(std::shared_ptr<PIDFile> file);
	void closeAllFiles();

	void openPidFileDialog();
	void addLibraryDialog();
	void addLibrary(std::filesystem::path& path, std::shared_ptr<SupportedGame> game);
	bool isFileAlreadyOpen(const std::filesystem::path& path, PIDFile** outFilePtr = NULL);
	void openAllFiles(std::shared_ptr<AssetLibrary::TreeNode> node);
};
