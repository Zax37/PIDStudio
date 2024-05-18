cmake_minimum_required(VERSION 3.22)

project(PIDStudio LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG")

set(SOURCE_FILES
    src/main.cpp
    src/PIDStudio.cpp
    src/File.cpp
    src/String.cpp
    src/PCXFile.cpp
    src/PIDFile.cpp
    src/PIDPalette.cpp
    src/AssetLibrary.cpp
    src/SupportedGame.cpp
    src/games/Claw.cpp
)

set(HEADER_FILES
    src/PIDStudio.h
    src/File.h
    src/String.h
    src/PCXFile.h
    src/PIDFile.h
    src/PIDPalette.h
    src/AssetLibrary.h
    src/SupportedGame.h
    src/games/Claw.h
)

set(RESOURCE_FILES
    resources/resources.rc
	src/assets/claw_game.pal.h
	src/assets/font.ttf.h
	src/assets/grayscale.pal.h
	src/assets/icon.png.h
	src/assets/IconsLucide.h
	src/assets/lucide.ttf.h
)

if(MSVC)
    add_compile_options(
        $<$<CONFIG:>:/MT> #---------|
        $<$<CONFIG:Debug>:/MTd> #---|-- Statically link the runtime libraries
        $<$<CONFIG:Release>:/MT> #--|
    )
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} /Od")
endif(MSVC)

find_package(SFML 2.5 COMPONENTS graphics window system CONFIG REQUIRED)
set(SFML_LIBS sfml-graphics sfml-system sfml-window)

option(IMGUI_SFML_ENABLE_VIEWPORTS "Enable ImGui viewport support" ON)
set(IMGUI_DIR ${PROJECT_SOURCE_DIR}/imgui)
add_subdirectory(imgui-sfml EXCLUDE_FROM_ALL)

find_package(Intl REQUIRED)
find_package(tinyfiledialogs REQUIRED)
find_package(efsw REQUIRED)

add_subdirectory(fmt EXCLUDE_FROM_ALL)

add_executable(${PROJECT_NAME} ${SOURCE_FILES} ${HEADERS} ${RESOURCE_FILES})
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

if (UNIX)
    set(THREADS_PREFER_PTHREAD_FLAG ON)
	find_package(Threads REQUIRED)
	target_link_libraries(${PROJECT_NAME} PRIVATE Threads::Threads)
endif (UNIX)

if(MSVC)
	set_target_properties(
		${PROJECT_NAME} PROPERTIES
		VS_DEBUGGER_WORKING_DIRECTORY "$(ProjectDir)..\\")
endif(MSVC)
