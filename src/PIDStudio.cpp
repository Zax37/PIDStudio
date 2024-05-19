#include "PIDStudio.h"

#include "PIDFile.h"
#include "PIDPalette.h"

#include "games/Claw.h"

#include <stack>
#include <fmt/core.h>
#include <libintl.h>
#include <tinyfiledialogs/tinyfiledialogs.h>

#include <imgui-SFML.h>
#include <SFML/Graphics.hpp>
#include "imgui_internal.h"

#include "assets/IconsLucide.h"
#include "assets/lucide.ttf.h"
#include "assets/icon.png.h"
#include "assets/font.ttf.h"
#include "assets/grayscale.pal.h"

#define _(String) gettext(String)
#define _STRINGS_TO_TRANSLATE_ _("Claw") _("Gruntz") _("Get Medieval")

#define SCREEN_WIDTH 1024
#define SCREEN_HEIGHT 768

const char APPLICATION_NAME[] = "PIDStudio";
const char SETTINGS_INI_FILENAME[] = "settings.ini";
const char ASSET_LIBRARIES_INI_KEY[] = "AssetLibraries";
const char ASSET_LIBRARY_WINDOW_ID[] = "###AssetLibrary";

// TODO: move out custom extensions to separate file?
namespace ImGui {
    inline void CenteredImage(const sf::Texture& texture)
    {
        ImVec2 windowSize = GetWindowSize();
        sf::Vector2u imageSize = texture.getSize();
        SetCursorPosX((windowSize.x - (float)imageSize.x) * 0.5f);
        SetCursorPosY((windowSize.y - (float)imageSize.y) * 0.5f + 10.0f);
        Image(texture);
    }

    bool BringFocusTo(ImGuiWindow* window)
    {
        if (!window || !window->DockNode || !window->DockNode->TabBar)
            return false;

        window->DockNode->TabBar->NextSelectedTabId = window->TabId;
        return true;
    }
}

PIDStudio::PIDStudio() : mainWindow(sf::VideoMode(SCREEN_WIDTH, SCREEN_HEIGHT), APPLICATION_NAME) {
    // initialize localization
    setlocale(LC_ALL, "");
    bindtextdomain(APPLICATION_NAME, "locale");
    textdomain(APPLICATION_NAME);
    bind_textdomain_codeset(APPLICATION_NAME, "UTF-8");

    // initialize fallback palette and list of supported games
    defaultPalette = std::make_shared<PIDPalette>(GRAYSCALE_PAL);
    claw = std::make_shared<Claw>(this, "Claw", "CLAW", "CLAW.EXE");
    supportedGames.emplace_back(claw);

    // initialize window with a custom icon
	sf::Image applicationIcon;
	applicationIcon.loadFromMemory(ICON_PNG, sizeof(ICON_PNG));
	mainWindow.setIcon(ICON_PNG_WIDTH, ICON_PNG_HEIGHT, applicationIcon.getPixelsPtr());
	mainWindow.setFramerateLimit(60);

	ImGui::SFML::Init(mainWindow, false);

    // configure UI library styles and flags
    ImGuiStyle* style = &ImGui::GetStyle();
    style->WindowMenuButtonPosition = ImGuiDir_None;

    auto& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigDockingWithShift = true;

    // load default font
    float baseFontSize = 16.0f;
    float iconFontSize = 13.0f;

    ImFontConfig fontConfig;
    fontConfig.FontDataOwnedByAtlas = false;
    static const ImWchar fontRanges[] =
    {
        0x0020, 0x00FF, // Basic Latin + Latin Supplement
        0x0100, 0x017F, // Extended Latin A
        0,
    };
    io.Fonts->AddFontFromMemoryTTF((void*)FONT_TTF, FONT_TTF_SIZE, baseFontSize, &fontConfig, fontRanges);

    // load icons font, merging to default font
    ImFontConfig iconsConfig;
    iconsConfig.FontDataOwnedByAtlas = false;
    iconsConfig.MergeMode = true;
    iconsConfig.PixelSnapH = true;
    iconsConfig.GlyphOffset.y = 1;
    static const ImWchar iconsRanges[] = { ICON_MIN_LC, ICON_MAX_LC, 0 };
    io.Fonts->AddFontFromMemoryTTF((void*)LUCIDE_TTF, LUCIDE_TTF_SIZE, iconFontSize, &iconsConfig, iconsRanges);

    // finalize loading fonts
    io.Fonts->Build();
    ImGui::SFML::UpdateFontTexture();

    // load settings from ini file
    mINI::INIFile file(SETTINGS_INI_FILENAME);
    file.read(settings);

    for (auto const& library : settings[ASSET_LIBRARIES_INI_KEY]) {
        for (auto const& game : supportedGames) {
            if (library.first == game->getIniKey()) {
                assetLibraries.emplace_back(std::make_shared<AssetLibrary>(this, library.second.c_str(), game));
                break;
            }
        }
    }
}

PIDStudio::~PIDStudio(){
    // save settings to ini file on application close
    mINI::INIFile file(SETTINGS_INI_FILENAME);
    file.write(settings);

    // properly close all UI stuff
	ImGui::SFML::Shutdown();
	mainWindow.close();
}

int PIDStudio::run(){
	sf::Clock deltaClock{};
    sf::Event event{};

    while (mainWindow.isOpen()) {
        // process window events
        while (mainWindow.pollEvent(event)) {
            ImGui::SFML::ProcessEvent(mainWindow, event);

            switch (event.type)
            {
            case sf::Event::Closed:
                mainWindow.close();
                break;
            case sf::Event::KeyPressed:
                if (event.key.control) {
                    switch (event.key.code)
                    {
                    case sf::Keyboard::O:
                        openPidFileDialog();
                        break;
                    case sf::Keyboard::W:
                        if (currentlyFocusedFile)
                            filesToClose.insert(currentlyFocusedFile);
                        break;
                    default:
                        break;
                    }
                }
                break;
            default:
                break;
            }
        }

        // main UI loop
        ImGui::SFML::Update(mainWindow, deltaClock.restart());

        const ImGuiViewport* main_viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x, main_viewport->WorkPos.y));
        ImGui::SetNextWindowSize(ImVec2(main_viewport->WorkSize.x, main_viewport->WorkSize.y));
        
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.f, 0.f));
        ImGui::Begin(APPLICATION_NAME, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoDocking);
        ImGui::PopStyleVar();

        menuBar();
        toolBar();
        preDockedWindows();
        openedFilesWindows();

        ImGui::End();

        mainWindow.clear();
        ImGui::SFML::Render(mainWindow);
        mainWindow.display();

        // update child windows undocked from main window
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();

        if (libraryToClose) {
            settings[ASSET_LIBRARIES_INI_KEY].remove(libraryToClose->getIniKey());
            assetLibraries.erase(std::find(assetLibraries.begin(), assetLibraries.end(), libraryToClose));
            libraryToClose.reset();
        }
        // react to filesystem changes
        for (auto const& library : assetLibraries) {
            library->rebuildTreeIfRequired();
        }
    }

    return 0;
}

void PIDStudio::menuBar()
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu(_("File")))
        {
            if (ImGui::BeginMenu(_("New..."))) {
                if (ImGui::MenuItemEx(_("Project"), ICON_LC_FOLDER_PLUS, "Ctrl+Shift+N", false, false)) { /* TODO */ }
                if (ImGui::MenuItemEx(_("File"), ICON_LC_IMAGE_PLUS, "Ctrl+N", false, false)) { /* TODO */ }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu(_("Open..."))) {
                if (ImGui::MenuItemEx(_("Project"), ICON_LC_FOLDER_UP, "Ctrl+Shift+O", false, false)) { /* TODO */ }
                if (ImGui::MenuItemEx(_("File"), ICON_LC_IMAGE_UP, "Ctrl+O")) { openPidFileDialog(); }
                ImGui::EndMenu();
            }

            ImGui::Separator();

            closeContextMenu();

            ImGui::Separator();

            if (ImGui::MenuItemEx(_("Save"), ICON_LC_SAVE, "Ctrl+S", false, false)) { /* TODO */ }
            saveAsContextMenu();
            if (ImGui::MenuItemEx(_("Save all"), ICON_LC_SAVE_ALL, "Ctrl+Shift+S", false, false)) { /* TODO */ }

            ImGui::Separator();

            if (ImGui::BeginMenuEx(_("Recently opened files"), nullptr, false)) {
                /* TODO */
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenuEx(_("Recently opened projects"), nullptr, false)) {
                /* TODO */
                ImGui::EndMenu();
            }

            ImGui::Separator();

            if (ImGui::MenuItem(_("Exit"), "Alt+F4")) { mainWindow.close(); }

            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}

void PIDStudio::toolBar()
{
    ImGui::PushStyleColor(ImGuiCol_Button, 0);

    if (ImGui::BeginMenuBar()) {
        ImGui::BeginDisabled();
        if (ImGui::Button(ICON_LC_IMAGE_PLUS)) { /* TODO */ }
        ImGui::EndDisabled();

        if (ImGui::Button(ICON_LC_IMAGE_UP)) { openPidFileDialog(); }

        ImGui::BeginDisabled();
        if (ImGui::Button(ICON_LC_SAVE)) { /* TODO */ }
        if (ImGui::Button(ICON_LC_SAVE_ALL)) { /* TODO */ }
        ImGui::EndDisabled();

        ImGui::EndMenuBar();
    }

    ImGui::PopStyleColor();
}

void PIDStudio::preDockedWindows()
{
    static bool shouldPrepareDockspace = true; // pre-dock windows on launch, but allow user to move them freely later
    dockspaceId = ImGui::DockSpace(ImGui::GetID(APPLICATION_NAME));

    if (shouldPrepareDockspace)
    {
        ImGui::DockBuilderRemoveNode(dockspaceId);
        ImGui::DockBuilderAddNode(
        dockspaceId,
        ImGuiDockNodeFlags_DockSpace | ImGuiDockNodeFlags_NoResizeX | ImGuiDockNodeFlags_NoResizeY
        );
        ImGui::DockBuilderSetNodeSize(dockspaceId, ImGui::GetWindowSize());

        // split initial UI layout to specific dockable areas
        dockspaceIdRight = ImGui::DockBuilderSplitNode(dockspaceId, ImGuiDir_Right, 0.25f, nullptr, &dockspaceIdLeft);
        dockspaceIdRightTop = ImGui::DockBuilderSplitNode(dockspaceIdRight, ImGuiDir_Up, 0.25f, nullptr, &dockspaceIdRightBottom);

        ImGui::SetNextWindowDockID(dockspaceIdRightTop, ImGuiDir_Up);
    }
    paletteWindow();

    if (shouldPrepareDockspace)
    {
        ImGui::SetNextWindowDockID(dockspaceIdRightTop, ImGuiDir_Up);
    }
    metadataWindow();

    if (shouldPrepareDockspace)
    {
        shouldPrepareDockspace = false;
        ImGui::SetNextWindowDockID(dockspaceIdRightBottom, ImGuiDir_Down);
        ImGui::DockBuilderFinish(dockspaceId);
    }
    libraryWindow();
}

void PIDStudio::openedFilesWindows()
{
    for (const std::shared_ptr<PIDFile>& file : openedFiles) {
        auto result = openedFileWindow(file);

        if (result == CLOSE) {
            filesToClose.insert(file);
        }
    }

    if (openedLibraryFile) {
        auto result = openedFileWindow(openedLibraryFile);

        switch (result) {
        case KEEP_OPEN:
            bringFocusTo = openedLibraryFile.get();
            openedFiles.push_back(openedLibraryFile);
            openedLibraryFile.reset();
            break;
        case CLOSE:
            filesToClose.insert(openedLibraryFile);
            break;
        default:
            break;
        }
    }

    if (!filesToClose.empty()) {
        for (const auto& file : filesToClose) {
            closeFile(file);
        }
        filesToClose.clear();
    }
}

PIDStudio::OPENED_FILE_WINDOW_RESULT PIDStudio::openedFileWindow(const std::shared_ptr<PIDFile>& file)
{
    bool isLibraryFile = file == openedLibraryFile;

    // Library files use same ID to make sure the window remains its position etc.
    std::string windowName = isLibraryFile ? "[L] " + file->getName() + ASSET_LIBRARY_WINDOW_ID : file->getWindowName();
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing;

    bool didNotCloseWindow = true, didClickKeepLibraryFileOpen = false; // outputs from UI library
    ImGui::Begin(windowName.c_str(), flags, &didNotCloseWindow, isLibraryFile ? &didClickKeepLibraryFileOpen : nullptr);
    if (ImGui::BeginPopupContextItem()) {
        closeContextMenu();
        ImGui::EndPopup();
    }

    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (ImGui::IsWindowAppearing()) {
        ImGui::SetWindowDock(window, dockspaceId, ImGuiDir_Right);
        currentlyFocusedFile = file;
    }
    if (ImGui::IsWindowFocused()) {
        currentPalette = file->getPalette();
    }
    if (ImGui::IsWindowDocked()) {
        // This fixes windows receiving input after clicking off external modal, which resulted in wrong window being focused.
        ImGui::GetCurrentWindow()->Flags |= ImGuiWindowFlags_NoMouseInputs;
    }
    ImGui::CenteredImage(file->getTexture());
    ImGui::End();

    if (file.get() == bringFocusTo) {
        if (ImGui::BringFocusTo(window)) {
            bringFocusTo = nullptr;
            currentPalette = file->getPalette();
            currentlyFocusedFile = file;
        }
    }

    return !didNotCloseWindow ? CLOSE : didClickKeepLibraryFileOpen ? KEEP_OPEN : NONE;
}

void PIDStudio::paletteWindow()
{
    if (ImGui::Begin(_("Palette"))) {
        std::shared_ptr<PIDPalette> palette = currentPalette ? currentPalette : defaultPalette;
        ImGui::CenteredImage(palette->getTexture());
    }
    ImGui::End();
}

void PIDStudio::metadataWindow()
{
    if (ImGui::Begin(_("Metadata"))) {
        if (currentlyFocusedFile) {
            int* userData = currentlyFocusedFile->getUserData();
            ImGui::Text("%s: %dx%d\n%s: %d, %d\n%s: %d, %d\n%s:\n%s",
                _("Image size"), currentlyFocusedFile->getWidth(), currentlyFocusedFile->getHeight(),
                _("Offset"), currentlyFocusedFile->getOffsetX(), currentlyFocusedFile->getOffsetY(),
                _("User values"), userData[0], userData[1],
                _("Flags"), currentlyFocusedFile->getFlagsDescription().c_str()
            );
        } else {
            ImGui::Text("%s", _("No opened files."));
        }
    }
    ImGui::End();
}

void PIDStudio::libraryWindow()
{
    ImGui::Begin(_("Library"));

    if (assetLibraries.empty()) {
        const char* text = _("No games in the library.");
        const char* label = _("Add a game");

        ImVec2 windowSize = ImGui::GetWindowSize();
        ImVec2 textSize = ImGui::CalcTextSize(text);
        ImVec2 labelSize = ImGui::CalcTextSize(label);

        ImGui::SetCursorPos(
            ImVec2(
                (windowSize.x - textSize.x) * 0.5f,
                (windowSize.y - textSize.y - labelSize.y) * 0.5f - 10.0f
            )
        );
        ImGui::Text("%s", text);

        ImGui::SetCursorPos(
            ImVec2(
                (windowSize.x - labelSize.x) * 0.5f,
                (windowSize.y - labelSize.y) * 0.5f + 15.0f
            )
        );

        if (ImGui::Button(label)) {
            addLibraryDialog();
        }
    } else {
        for (const std::shared_ptr<AssetLibrary>& library : assetLibraries)
            library->displayContent();
    }

    ImGui::End();
}

void PIDStudio::openPidFileDialog()
{
    const char* filterPattern = "*.pid";
    const char* filterName = "PID Files";
    const char* selectedFiles = tinyfd_openFileDialog(
        _("Open PID file(s)"),
        nullptr, // default path
        1, &filterPattern, filterName,
        1 // multi select
    );

    if (selectedFiles) {
        std::istringstream stream(selectedFiles);
        std::string filePath;

        while (std::getline(stream, filePath, '|')) {
            std::filesystem::path path(filePath);
            if (!isFileAlreadyOpen(path)) {
                std::shared_ptr<AssetLibrary::TreeNode> libraryFileNode;
                for (auto& library : assetLibraries) {
                    if (library->hasFilepath(path, libraryFileNode)) {
                        openLibraryFile(library, libraryFileNode);
                        break;
                    }
                }

                if (!libraryFileNode)
                {
                    std::shared_ptr<PIDFile> file = std::make_shared<PIDFile>(this);
                    if (file->loadFromFile(path)) {
                        bringFocusTo = file.get();
                        openedFiles.emplace_back(file);
                    }
                }
            }
        }
    }
}

void PIDStudio::addLibraryDialog()
{
    const char* selectedFolder = tinyfd_selectFolderDialog(_("Select game directory"), nullptr);

    if (!selectedFolder) return;

    std::filesystem::path path = selectedFolder;

    if (!std::filesystem::exists(path / claw->getExeName())) {
        path /= "..";
    }

    if (std::filesystem::exists(path / claw->getExeName())) {
        path /= "Assets";

        if (!std::filesystem::exists(path)) {
            tinyfd_messageBox(_("No Assets directory"), _("Looks like you are trying to use older version of Claw where assets were packed inside CLAW.REZ file. Please update to CrazyHook version."), "ok", "error", 1);
            return;
        }

        addLibrary(path, claw);
        return;
    }

    std::string message = fmt::format("{} {}.", _("Looks like you are trying to add a game that is not supported. Currently supported games:"), SupportedGame::getNames(supportedGames));
    tinyfd_messageBox(_("Game not recognized"), message.c_str(), "ok", "error", 1);
}

void PIDStudio::libraryEntryContextMenu(const std::shared_ptr<AssetLibrary>& library, const std::shared_ptr<AssetLibrary::TreeNode>& node, bool isLeaf, bool isRoot)
{
    if (isLeaf) {
        saveAsContextMenu();
    } else {
        if (isRoot) {
            if (ImGui::MenuItem(_("Remove from library"))) { libraryToClose = library; }
            ImGui::Separator();
        }
        if (ImGui::MenuItem(_("Open all"))) { openAllFiles(library, node); }
    }
}

void PIDStudio::addLibrary(std::filesystem::path& path, const std::shared_ptr<SupportedGame>& game)
{
    std::string pathString = path.string();
    settings[ASSET_LIBRARIES_INI_KEY][game->getIniKey()] = pathString;
    assetLibraries.emplace_back(std::make_shared<AssetLibrary>(this, path, game));

    // check if we can infer palette for one of already opened files from the newly added library
    for (const auto& file : openedFiles) {
        if (file->getPalette()) continue;

        std::shared_ptr<AssetLibrary::TreeNode> outFoundNode;
        if (assetLibraries.back()->hasFilepath(file->getPath(), outFoundNode)) {
            file->setPalette(assetLibraries.back()->inferPalette(outFoundNode));
            if (file == currentlyFocusedFile) {
                currentPalette = file->getPalette();
            }
        }
    }
}

void PIDStudio::closeContextMenu()
{
    bool isAnyTabOpen = false;
    std::string closeFile = _("Close");
    if (currentlyFocusedFile) {
        isAnyTabOpen = true;
        closeFile += ' ' + currentlyFocusedFile->getName();
    }

    if (ImGui::MenuItem(closeFile.c_str(), "Ctrl+W", false, isAnyTabOpen)) { filesToClose.insert(currentlyFocusedFile); }
    if (ImGui::MenuItem(_("Close all"), "Ctrl+Shift+W", false, isAnyTabOpen)) { closeAllFiles(); }
}

void PIDStudio::saveAsContextMenu()
{
    if (ImGui::MenuItemEx(_("Save as..."), nullptr, nullptr, false, false)) { /* Do stuff */ }
}

void PIDStudio::closeFile(const std::shared_ptr<PIDFile>& file)
{
    if (file == openedLibraryFile) {
        openedLibraryFile.reset();
    } else {
        openedFiles.erase(std::find(openedFiles.begin(), openedFiles.end(), file));
    }

    if (currentlyFocusedFile == file) currentlyFocusedFile.reset();
}

void PIDStudio::closeAllFiles()
{
    for (const auto& file : openedFiles) {
        filesToClose.insert(file);
    }

    if (openedLibraryFile) {
        filesToClose.insert(openedLibraryFile);
    }
}

void PIDStudio::openLibraryFile(const std::shared_ptr<AssetLibrary>& library, const std::shared_ptr<AssetLibrary::TreeNode>& node, bool inSeparateWindow)
{
    PIDFile* openedFile;
    if (isFileAlreadyOpen(node->path, &openedFile)) {
        bringFocusTo = openedFile;
        return;
    }

    auto file = std::make_shared<PIDFile>(this);
    if (file->loadFromFile(node->path)) {
        openedLibraryFile = file;
        bringFocusTo = file.get();

        std::shared_ptr<PIDPalette> palette = library->inferPalette(node);

        if (palette) {
            openedLibraryFile->setPalette(palette);
            currentPalette = palette;
        }

        if (inSeparateWindow) {
            openedFiles.emplace_back(openedLibraryFile);
            openedLibraryFile.reset();
        }
    }
}

bool PIDStudio::isFileAlreadyOpen(const std::filesystem::path& path, PIDFile** outFilePtr)
{
    for (auto& file : openedFiles) {
        if (file->getPath() == path) {
            if (outFilePtr) {
                *outFilePtr = file.get();
            }
            return true;
        }
    }

    return false;
}

void PIDStudio::openAllFiles(const std::shared_ptr<AssetLibrary>& library, const std::shared_ptr<AssetLibrary::TreeNode>& selectedNode)
{
    std::stack<std::shared_ptr<AssetLibrary::TreeNode>> stack;
    stack.emplace(selectedNode);

    while(!stack.empty()) {
        const auto node = stack.top();
        stack.pop();

        for (const auto &entry: node->children) {
            if (entry->path.empty()) {
                stack.emplace(entry);
            } else {
                openLibraryFile(library, entry, true);
            }
        }
    }
}
