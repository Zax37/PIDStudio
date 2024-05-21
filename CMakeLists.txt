cmake_minimum_required(VERSION 3.22)

project(PIDStudio VERSION 1.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG")

set(
    SOURCE_FILES
    src/main.cpp
    src/PIDStudio.cpp
    src/PIDStudio.h
    src/File.cpp
    src/File.h
    src/Filters.tpp
    src/Filters.h
    src/String.cpp
    src/String.h
    src/ImGuiExtensions.cpp
    src/ImGuiExtensions.h
    src/formats/PCXFile.cpp
    src/formats/PCXFile.h
    src/formats/PIDFile.cpp
    src/formats/PIDFile.h
    src/formats/PIDPalette.cpp
    src/formats/PIDPalette.h
    src/AssetLibrary.cpp
    src/AssetLibrary.h
    src/SupportedGame.cpp
    src/SupportedGame.h
    src/games/Claw.cpp
    src/games/Claw.h
)

set(
    RESOURCE_FILES
    resources/resources.rc
    src/assets/claw_game.pal.h
    src/assets/font.ttf.h
    src/assets/grayscale.pal.h
    src/assets/icon.png.h
    src/assets/IconsLucide.h
    src/assets/lucide.ttf.h
)

if (MSVC)
    add_compile_options(
        $<$<CONFIG:>:/MT> #---------|
        $<$<CONFIG:Debug>:/MTd> #---|-- Statically link the runtime libraries
        $<$<CONFIG:Release>:/MT> #--|
    )
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} /Od /Zi /JMC")
endif (MSVC)

find_package(SFML 2.5 COMPONENTS graphics window system CONFIG REQUIRED)
set(SFML_LIBS sfml-graphics sfml-system sfml-window)

if (APPLE)
else ()
    option(IMGUI_SFML_ENABLE_VIEWPORTS "Enable ImGui viewport support" ON)
endif (APPLE)
set(IMGUI_DIR ${PROJECT_SOURCE_DIR}/imgui)
add_subdirectory(imgui-sfml EXCLUDE_FROM_ALL)

find_package(Intl REQUIRED)
find_package(tinyfiledialogs REQUIRED)
find_package(efsw REQUIRED)

add_subdirectory(fmt EXCLUDE_FROM_ALL)

if (APPLE)
    set(PROJECT_FLAGS MACOSX_BUNDLE)
    set(MACOSX_BUNDLE_ICON_FILE "${CMAKE_PROJECT_NAME}.icns")
    set(application_icon "${CMAKE_SOURCE_DIR}/resources/${MACOSX_BUNDLE_ICON_FILE}")
    set_source_files_properties(${application_icon} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
    list(APPEND RESOURCE_FILES "${application_icon}")
endif (APPLE)

add_executable(${PROJECT_NAME} ${PROJECT_FLAGS} ${SOURCE_FILES} ${RESOURCE_FILES})
target_link_libraries(
    ${PROJECT_NAME} PRIVATE
    ${SFML_LIBS}
    ImGui-SFML::ImGui-SFML
    Intl::Intl
    tinyfiledialogs::tinyfiledialogs
    efsw::efsw
    fmt::fmt
)
target_include_directories(
    ${PROJECT_NAME} PRIVATE
    IconFontCppHeaders
    mINI/src
)

if (APPLE)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        BUNDLE TRUE
        MACOSX_BUNDLE_BUNDLE_NAME ${PROJECT_NAME}
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.zax37.${PROJECT_NAME}"
        MACOSX_BUNDLE_BUNDLE_VERSION ${CMAKE_PROJECT_VERSION}
        MACOSX_BUNDLE_SHORT_VERSION_STRING ${CMAKE_PROJECT_VERSION}
        )
endif (APPLE)

if (UNIX)
    set(THREADS_PREFER_PTHREAD_FLAG ON)
    find_package(Threads REQUIRED)
    target_link_libraries(${PROJECT_NAME} PRIVATE Threads::Threads)
endif (UNIX)

if (MSVC)
    set_target_properties(
        ${PROJECT_NAME} PROPERTIES
        VS_DEBUGGER_WORKING_DIRECTORY "$(ProjectDir)..\\")
endif (MSVC)

install(TARGETS ${PROJECT_NAME} BUNDLE DESTINATION ${CMAKE_INSTALL_BINDIR})
