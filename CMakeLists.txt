cmake_minimum_required(VERSION 3.10)

# Set SDL2 include path before project() for it to take effect
if(EXISTS "/opt/homebrew/include/SDL2")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I/opt/homebrew/include/SDL2")
elseif(EXISTS "/usr/local/include/SDL2")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I/usr/local/include/SDL2")
endif()

project(DarkEdenClient VERSION 1.0.0)

# C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Note: We do NOT define _DEBUG here because it conflicts with DebugInfo.h
# Instead, rely on the new DebugLog.h system which is always available
# Debug builds will have console output enabled via log_init() in GameInit.cpp

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Sanitizer configuration
option(USE_ASAN "Enable AddressSanitizer for memory error detection" OFF)
option(USE_TSAN "Enable ThreadSanitizer for race condition detection" OFF)
option(USE_UBSAN "Enable UndefinedBehaviorSanitizer for undefined behavior detection" OFF)

if(USE_ASAN OR USE_TSAN OR USE_UBSAN)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        set(SANITIZER_FLAGS "")

        if(USE_ASAN)
            set(SANITIZER_FLAGS "${SANITIZER_FLAGS} -fsanitize=address -fno-omit-frame-pointer")
            set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address")
        endif()

        if(USE_TSAN)
            set(SANITIZER_FLAGS "${SANITIZER_FLAGS} -fsanitize=thread")
            set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=thread")
        endif()

        if(USE_UBSAN)
            set(SANITIZER_FLAGS "${SANITIZER_FLAGS} -fsanitize=undefined")
            set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=undefined")
        endif()

        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${SANITIZER_FLAGS}")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${SANITIZER_FLAGS}")

        message(STATUS "Sanitizer flags: ${SANITIZER_FLAGS}")
    endif()
endif()

# SDL backend is now mandatory on all platforms (Windows native support removed)
set(USE_SDL_BACKEND ON CACHE BOOL "Use SDL backend instead of native Windows APIs" FORCE)

option(BUILD_ENGINE "Build new SDL-based engine" ON)

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Add SDL2 include directory BEFORE add_subdirectory
if(EXISTS "/opt/homebrew/include/SDL2")
	include_directories(/opt/homebrew/include/SDL2)
elseif(EXISTS "/usr/local/include/SDL2")
	include_directories(/usr/local/include/SDL2)
endif()

# Find SDL2 (needed for engine and DXLib)
# Skip for Emscripten - it uses -s USE_SDL=2 linker flags instead
if(BUILD_ENGINE OR USE_SDL_BACKEND OR NOT WIN32)
	if(EMSCRIPTEN)
		# Emscripten provides SDL2 through its port system
		message(STATUS "Emscripten build: SDL2 will be provided through linker flags")
		# Add linker flags globally for all executables
		add_link_options("-sUSE_SDL=2" "-sUSE_SDL_IMAGE=2" "-sUSE_SDL_TTF=2")
		# Create interface targets for compatibility with empty link libraries
		add_library(SDL2_interface INTERFACE IMPORTED)
		add_library(SDL2_image_interface INTERFACE IMPORTED)
		add_library(SDL2_ttf_interface INTERFACE IMPORTED)
		add_library(SDL2main_interface INTERFACE IMPORTED)

		# Create aliases for compatibility
		add_library(SDL2::SDL2 ALIAS SDL2_interface)
		add_library(SDL2::SDL2main ALIAS SDL2main_interface)
		add_library(SDL2_image::SDL2_image ALIAS SDL2_image_interface)
		add_library(SDL2_ttf::SDL2_ttf ALIAS SDL2_ttf_interface)
	else()
		find_package(SDL2 REQUIRED)
		find_package(SDL2_image REQUIRED)
		find_package(SDL2_ttf REQUIRED)
	endif()
endif()

# Find iconv library (needed for MString.cpp/TextService.cpp). On Windows
# this isn't a system library - it comes from vcpkg - so it needs an actual
# find_package() to resolve a full path; a bare target_link_libraries(iconv)
# just passes the literal string "iconv" to the linker, which then can't
# find iconv.lib in its default search paths (LNK1104) even though it's
# right there in the vcpkg install.
find_package(Iconv QUIET)

# Find JPEG library (needed for UtilityFunction.cpp)
find_package(JPEG QUIET)
if(NOT JPEG_FOUND)
	# Try pkg-config as fallback
	find_package(PkgConfig QUIET)
	if(PKG_CONFIG_FOUND)
		pkg_check_modules(JPEG libjpeg)
	endif()
endif()
if(JPEG_FOUND OR JPEG_LIBRARIES)
	message(STATUS "JPEG library found: ${JPEG_LIBRARIES}")
else()
	message(WARNING "JPEG library not found - JPG loading/saving may not work")
endif()

# Subdirectories
add_subdirectory(basic)
add_subdirectory(Client/DXLib)
add_subdirectory(Client/SpriteLib)

# Additional libraries from VC6 project
add_subdirectory(Client/framelib)

# ============================================================
# TextSystem - New SDL-based Text System (Abstracted)
# ============================================================
if(USE_SDL_BACKEND OR NOT WIN32)
	set(TEXTSYSTEM_SOURCES
		Client/TextSystem/TextService.cpp
		Client/TextSystem/TextBackendSDL.cpp
		Client/TextSystem/RenderTargetSpriteSurface.cpp
	)

	add_library(TextSystem STATIC ${TEXTSYSTEM_SOURCES})

	target_include_directories(TextSystem PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}/Client/TextSystem
		${CMAKE_CURRENT_SOURCE_DIR}/Client/SpriteLib
		${CMAKE_CURRENT_SOURCE_DIR}/Client/DXLib
		${CMAKE_CURRENT_SOURCE_DIR}/Client
		${CMAKE_CURRENT_SOURCE_DIR}/basic
		${SDL2_INCLUDE_DIRS}
	)

	target_link_libraries(TextSystem PUBLIC
		SpriteLib
		SDL2::SDL2
		SDL2_ttf::SDL2_ttf
	)

	target_compile_definitions(TextSystem PRIVATE SPRITELIB_BACKEND_SDL USE_SDL_BACKEND)
	if(NOT WIN32)
		target_compile_definitions(TextSystem PRIVATE PLATFORM_MACOS)
	endif()

	message(STATUS "TextSystem library will be built from ${TEXTSYSTEM_SOURCES}")
endif()

if(BUILD_ENGINE)
	add_subdirectory(tools/engine/sprite)
endif()

# ============================================================
# VS_UI Library - UI System (按需编译必要的 Client 文件)
# ============================================================
if(USE_SDL_BACKEND OR NOT WIN32)
	# VS_UI source files
	file(GLOB_RECURSE VS_UI_SRC_SOURCES
		VS_UI/src/*.cpp
		VS_UI/*.cpp  # Include VS_UI root directory for RarFile.cpp
	)

	# Exclude Windows-specific files
	if(NOT WIN32)
		list(FILTER VS_UI_SRC_SOURCES EXCLUDE REGEX ".*Imm.*")
		list(FILTER VS_UI_SRC_SOURCES EXCLUDE REGEX ".*WinMain.*")
		list(FILTER VS_UI_SRC_SOURCES EXCLUDE REGEX ".*WebBrowser.*")
		# U_edit is now cross-platform (SDL-based), so don't exclude it
		# Exclude problematic files in VS_UI root directory
		list(FILTER VS_UI_SRC_SOURCES EXCLUDE REGEX "VS_UI/DebugInfo\\.cpp")
		list(FILTER VS_UI_SRC_SOURCES EXCLUDE REGEX "VS_UI/MitemTableInit\\.cpp")
		list(FILTER VS_UI_SRC_SOURCES EXCLUDE REGEX "VS_UI/SXml\\.cpp")
		# Note: u_window.cpp is now cross-platform compatible
	endif()

	# Exclude native Win32 GDI/IME implementations superseded by their SDL-backend
	# replacements (Ci_macOS.cpp, Client/RenderingFunctions.cpp). USE_SDL_BACKEND is
	# forced ON above for all platforms including WIN32 ("Windows native support
	# removed"), so these must be excluded regardless of platform, not just NOT WIN32.
	if(USE_SDL_BACKEND)
		list(FILTER VS_UI_SRC_SOURCES EXCLUDE REGEX "VS_UI/src/hangul/Ci\\.cpp")
		list(FILTER VS_UI_SRC_SOURCES EXCLUDE REGEX "VS_UI/src/hangul/FL2\\.cpp")
	endif()

	# Exclude VS_UI/WinMain.cpp on WIN32 too (already excluded above for NOT WIN32).
	# It defines its own WinMain/InitApp/WindowProc/g_hWnd/g_GameRect/UI_ResultReceiver
	# etc., which duplicate symbols already defined by Client/Client.cpp and
	# Client/GameUI.cpp - the actual, functionally complete entry point (it alone
	# calls InitGame()/g_pTopView->Init()/g_pUpdate, which Client/SDLMain.cpp's own
	# header comment confirms it was ported from: "Based on WinMain from Client.cpp").
	# VS_UI/WinMain.cpp never calls InitGame() at all - it only drives the VS_UI
	# widget system (gC_vs_ui.Process()/Show()), so even where it happened to link
	# it would never start the actual game. See
	# 참고자료/커밋로그/2026-08-21_VS_UI_WinMain_cpp_죽은_진입점_제외.md.
	if(WIN32)
		list(FILTER VS_UI_SRC_SOURCES EXCLUDE REGEX "VS_UI/WinMain\\.cpp")
	endif()

	# Client files that VS_UI depends on (按需编译)
	set(VS_UI_CLIENT_SOURCES
		# Core
		Client/Client.cpp
		Client/ClientFunction.cpp

		# Item System
		Client/MItem.cpp
		Client/MItemManager.cpp
		Client/MItemOptionTable.cpp
		Client/MMoneyManager.cpp

		# Skill System
		Client/MSkillManager.cpp
		Client/SkillDef.cpp

		# Gear System
		Client/MSlayerGear.cpp
		Client/MVampireGear.cpp
		Client/MOustersGear.cpp
		Client/MQuickSlot.cpp

		# Inventory System
		Client/MInventory.cpp
		Client/MStorage.cpp
		Client/MShopShelf.cpp

		# Zone System
		Client/MZone.cpp
		Client/MZoneTable.cpp

		# Player System
		Client/MPlayer.cpp
		Client/MCreatureTable.cpp
		Client/MFakeCreature.cpp
		Client/MNpcTable.cpp
		Client/MNPCTable.cpp

		# Social System
		Client/MParty.cpp
		Client/MGuildMarkManager.cpp
		Client/MGuildInfoMapper.cpp

		# Trade System
		Client/MTradeManager.cpp
		Client/MPriceManager.cpp
		Client/MShop.cpp

		# Game Data
		Client/MGameStringTable.cpp
		Client/ExperienceTable.cpp
		Client/UserInformation.cpp
		Client/SystemAvailabilities.cpp
		Client/FameInfo.cpp
		Client/DebugInfo.cpp
		Client/ClientConfig.cpp
		Client/UtilityFunction.cpp
		# GameFunctions.cpp - Moved to CLIENT_MAIN_SOURCES to avoid duplicates with GameMain.cpp
		# GamePacketFunctions.cpp - Removed, all functions now in PacketFunction.cpp
		Client/RenderingFunctions.cpp
		Client/UIUtilityFunctions.cpp

		# Events & Time
		Client/MEventManager.cpp
		Client/MGameTime.cpp
		Client/MTimeItemManager.cpp

		# UI Support
		Client/MHelpMessageManager.cpp
		Client/MSoundTable.cpp
		Client/MMonsterKillQuestInfo.cpp
	)

	# Combine all sources
	set(VS_UI_ALL_SOURCES ${VS_UI_SRC_SOURCES} ${VS_UI_CLIENT_SOURCES})

	# Create VS_UI static library
	add_library(VS_UI STATIC ${VS_UI_ALL_SOURCES})

	# VS_UI_WebBrowser.h/.cpp use ATL (atlbase.h, CComBSTR, CComVariant) for the
	# legacy in-game IE browser control. ATL ships alongside the MSVC toolset
	# under atlmfc/include but is not on the default include path. Locate it
	# by searching installed VS instances directly rather than trusting
	# CMAKE_CXX_COMPILER, which can point at a toolset instance (e.g. a
	# CMakeCache generated against a since-removed VS Build Tools install)
	# that no longer matches whichever MSBuild is actually used to build.
	if(MSVC)
		file(GLOB _atlmfc_candidates
			"C:/Program Files/Microsoft Visual Studio/*/*/VC/Tools/MSVC/*/atlmfc/include"
			"C:/Program Files (x86)/Microsoft Visual Studio/*/*/VC/Tools/MSVC/*/atlmfc/include"
		)
		if(_atlmfc_candidates)
			list(SORT _atlmfc_candidates)
			list(GET _atlmfc_candidates -1 _atlmfc_include_dir)
			# ATL's headers (atlbase.h et al.) embed a
			# #pragma comment(lib, "atls.lib") - the linker needs this
			# directory too (DarkEden.exe, the final link target that
			# actually pulls in VS_UI, is set up further below), or it
			# can't find atls.lib even though the include dir above lets
			# it compile fine (LNK1104). Derive it from the same toolset
			# instance as the include dir above, not just any installed
			# atlmfc/lib, so headers and binary stay from the same version.
			string(REPLACE "/include" "/lib" _atlmfc_lib_dir "${_atlmfc_include_dir}")
			if(CMAKE_SIZEOF_VOID_P EQUAL 8)
				set(_atlmfc_lib_dir "${_atlmfc_lib_dir}/x64")
			else()
				set(_atlmfc_lib_dir "${_atlmfc_lib_dir}/x86")
			endif()
		endif()
	endif()

	# Include directories
	target_include_directories(VS_UI PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}/VS_UI
		${CMAKE_CURRENT_SOURCE_DIR}/VS_UI/src/header
		${CMAKE_CURRENT_SOURCE_DIR}/VS_UI/src/widget
		${CMAKE_CURRENT_SOURCE_DIR}/VS_UI/src/hangul
		${CMAKE_CURRENT_SOURCE_DIR}/VS_UI/src/Imm
		${CMAKE_CURRENT_SOURCE_DIR}/Client
		${CMAKE_CURRENT_SOURCE_DIR}/Client/TextSystem
		${CMAKE_CURRENT_SOURCE_DIR}/Client/SpriteLib
		${CMAKE_CURRENT_SOURCE_DIR}/Client/DXLib
		${CMAKE_CURRENT_SOURCE_DIR}/Client/framelib
		${CMAKE_CURRENT_SOURCE_DIR}/Client/Packet
		${CMAKE_CURRENT_SOURCE_DIR}/Client/WinLib
		${CMAKE_CURRENT_SOURCE_DIR}
		${_atlmfc_include_dir}
	)

	# Link libraries
	target_link_libraries(VS_UI PUBLIC
		basic
		dxlib
		SpriteLib
		framelib
		TextSystem
		SDL2::SDL2
		SDL2_image::SDL2_image
		SDL2_ttf::SDL2_ttf
	)

	# See DarkEden target below: SDL2main collides with Client/Client.cpp's own
	# WinAPI WinMain on Windows (VS_UI/WinMain.cpp, which used to duplicate this
	# reasoning, is now excluded from the build - see the WIN32 filter above), so
	# only link it (PUBLIC, so it still reaches consumers) on platforms where it
	# isn't a real Windows entry point.
	if(NOT WIN32)
		target_link_libraries(VS_UI PUBLIC SDL2::SDL2main)
	endif()

	# Platform-specific definitions
	if(NOT WIN32)
		target_compile_definitions(VS_UI PRIVATE PLATFORM_MACOS)
		# Also pass PLATFORM_MACOS to Client files compiled into VS_UI
		target_compile_definitions(VS_UI PUBLIC PLATFORM_MACOS)
	endif()

	# Define _LIB to match original VC6 build configuration
	# This excludes test/debug code wrapped in #ifndef _LIB from library builds
	target_compile_definitions(VS_UI PRIVATE _LIB)

	message(STATUS "VS_UI library will be built from ${VS_UI_SRC_SOURCES} VS_UI files + ${VS_UI_CLIENT_SOURCES} Client files")
endif()

# ============================================================
# Resource Management Tools
# ============================================================

# Resource Validator - checks all resource files for integrity
add_executable(resource_validator
	Client/tools/resource_validator.cpp
)

target_include_directories(resource_validator PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}
)

set_target_properties(resource_validator PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# ============================================================
# Resource Viewing Tools
# ============================================================

# Sprite Viewer - view .spk sprite pack files (C++ version using Client/SpriteLib)
add_executable(sprite_viewer
	tools/viewers/sprite_viewer/main.cpp
)

target_include_directories(sprite_viewer PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/Client/SpriteLib
	${CMAKE_CURRENT_SOURCE_DIR}/Client/DXLib
)

target_link_libraries(sprite_viewer PRIVATE
	SpriteLib
	dxlib
	basic
	SDL2::SDL2
)

set_target_properties(sprite_viewer PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)


# Zone Parser - parse and dump .map zone files (standalone C++ version)
add_executable(zone_parser
	tools/viewers/zone_parser/main.cpp
)

target_link_libraries(zone_parser PRIVATE)

set_target_properties(zone_parser PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Item Viewer - view .ispk indexed sprite pack files (C++ version using Client/SpriteLib)
add_executable(item_viewer
	tools/viewers/item_viewer/main.cpp
)

target_include_directories(item_viewer PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/Client/SpriteLib
	${CMAKE_CURRENT_SOURCE_DIR}/Client/DXLib
)

target_link_libraries(item_viewer PRIVATE
	SpriteLib
	dxlib
	basic
	SDL2::SDL2
)

set_target_properties(item_viewer PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Creature Viewer - view creature animations (C++ version using Client/SpriteLib + Client/framelib)
add_executable(creature_viewer
	tools/viewers/creature_viewer/main.cpp
)

target_include_directories(creature_viewer PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/Client/SpriteLib
	${CMAKE_CURRENT_SOURCE_DIR}/Client/DXLib
	${CMAKE_CURRENT_SOURCE_DIR}/Client/framelib
)

target_link_libraries(creature_viewer PRIVATE
	SpriteLib
	framelib
	dxlib
	basic
	SDL2::SDL2
)

set_target_properties(creature_viewer PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Map Viewer - browse zone maps with pan/zoom (C++ version using Client/SpriteLib)
add_executable(map_viewer
	tools/viewers/map_viewer/main.cpp
	tools/viewers/map_viewer/zoneloader.cpp
	Client/TileRenderer.cpp
)

target_include_directories(map_viewer PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/Client
	${CMAKE_CURRENT_SOURCE_DIR}/Client/SpriteLib
	${CMAKE_CURRENT_SOURCE_DIR}/Client/DXLib
	${CMAKE_CURRENT_SOURCE_DIR}/tools/engine/sprite/include
)

target_link_libraries(map_viewer PRIVATE
	sprite
	SpriteLib
	dxlib
	basic
	SDL2::SDL2
)

set_target_properties(map_viewer PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Effect Viewer - test and debug effect system
add_executable(effect_viewer
	tools/viewers/effect_viewer/main.cpp
	tools/viewers/effect_viewer/EffectSpriteTypeTable.cpp
	tools/viewers/effect_viewer/stubs.cpp
	Client/MObject.cpp
	Client/EffectResourceContainer.cpp
	Client/MEffect.cpp
)

target_include_directories(effect_viewer PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/Client
	${CMAKE_CURRENT_SOURCE_DIR}/Client/SpriteLib
	${CMAKE_CURRENT_SOURCE_DIR}/Client/DXLib
	${CMAKE_CURRENT_SOURCE_DIR}/Client/framelib
	${CMAKE_CURRENT_SOURCE_DIR}/basic
)

target_link_libraries(effect_viewer PRIVATE
	sprite
	SpriteLib
	framelib
	dxlib
	basic
	SDL2::SDL2
)

# Set output directory for effect_viewer
# Use CMAKE_BINARY_DIR to avoid nesting (build/debug-asan/bin/)
if(CMAKE_BUILD_TYPE STREQUAL "debug-asan")
	set_target_properties(effect_viewer PROPERTIES
		RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
	)
else()
	set_target_properties(effect_viewer PROPERTIES
		RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
	)
endif()


# Examples (always built if backend is enabled)
# Examples directory removed during cleanup
# if(USE_SDL_BACKEND OR NOT WIN32)
# 	add_subdirectory(examples)
# endif()

# Dark Eden Client Executable
if(USE_SDL_BACKEND OR NOT WIN32)
	# Collect remaining Client source files (excluding those already in VS_UI)
	file(GLOB CLIENT_MAIN_SOURCES
		Client/*.cpp
		Client/Packet/*.cpp
		Client/Packet/**/*.cpp
		Client/SXml/*.cpp
		Client/WinLib/*.cpp
	)

	# Add additional source files
	list(APPEND CLIENT_MAIN_SOURCES
		Client/COGGSTREAM.cpp
		# Time functions extracted from PacketFunction.cpp (which has Windows-specific networking code)
		Client/ClientTimeFunctions.cpp
		# ActionFunctions.cpp excluded - functions now in PacketFunction.cpp
		# Client/ActionFunctions.cpp
		# Globals.cpp - Global variables and utility functions (from GamePacketFunctions.cpp)
		Client/Globals.cpp
	)

	# Remove files that are already compiled in VS_UI library
	list(REMOVE_ITEM CLIENT_MAIN_SOURCES
		${VS_UI_CLIENT_SOURCES}
		# Explicitly remove files that duplicate globals in Client.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/Client/GlobalVariables.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/Client/GCNotifyWinHandler.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/Client/GCNotifyWin.cpp
		# Exclude MitemTableinit2.cpp - duplicates InitItem2() from MitemTableInit.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/Client/MitemTableinit2.cpp
		# Exclude MissingGlobals.cpp - duplicates globals with GameMain.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/Client/MissingGlobals.cpp
		# Exclude GameHelpers.cpp - duplicates many functions with GamePacketFunctions.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/Client/GameHelpers.cpp
		# Exclude GameFunctions.cpp - duplicates functions with GameMain.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/Client/GameFunctions.cpp
		# Exclude GamePacketFunctions.cpp - all functions now in PacketFunction.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/Client/GamePacketFunctions.cpp
		# Exclude ActionFunctions.cpp - functions now in PacketFunction.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/Client/ActionFunctions.cpp
	)

	# Note: Cpackets contains CG (Client to Game) handlers
	# These are server-side handlers that shouldn't be compiled in the client
	# The client-side packet handling is done through different mechanisms
	list(FILTER CLIENT_MAIN_SOURCES EXCLUDE REGEX ".*Cpackets/.*Handler\\.cpp")

	# Exclude CAvi.cpp: legacy MCI/AVFile-based intro-movie player (uses
	# MCI_DGV_* digital-video structures that current Windows SDKs no longer
	# declare in <mmsystem.h>/<vfw.h>). Confirmed dead: the only class it
	# defines (CAvi) is never instantiated - its two other includers,
	# GlobalVariables.cpp and MissingGlobals.cpp, are both already excluded
	# above via REMOVE_ITEM for duplicating globals with Client.cpp/GameMain.cpp.
	list(FILTER CLIENT_MAIN_SOURCES EXCLUDE REGEX ".*CAvi\\.cpp")

	# Exclude root-level Client/soundbuf.cpp and Client/mp3.cpp: duplicates of
	# Client/DXLib/soundbuf.cpp and Client/DXLib/mp3.cpp (already compiled into
	# the dxlib library that DarkEden links). The original VC6 project
	# (client-master_vs6/Client/DXLib/DXLib.dsp) only ever compiled these two
	# files as part of DXLib.lib - Client.dsp (the EXE) never listed them
	# directly. The root-level copies here still include the real
	# <dsound.h>/<wtypes.h> (unlike the DXLib/ versions, already migrated to
	# basic/AudioTypes.h), so the file(GLOB Client/*.cpp) above picking them up
	# both reintroduces the real-DirectSound redefinition conflict and risks
	# LNK2005 duplicate symbols against the dxlib copies once compiled cleanly.
	list(FILTER CLIENT_MAIN_SOURCES EXCLUDE REGEX "/Client/soundbuf\\.cpp$")
	list(FILTER CLIENT_MAIN_SOURCES EXCLUDE REGEX "/Client/mp3\\.cpp$")

	# Same duplication as soundbuf.cpp/mp3.cpp above: reader.cpp/synfilt.cpp/
	# subdecoder.cpp were also DXLib.lib-only in the original VC6 project
	# (client-master_vs6/Client/DXLib/DXLib.dsp) and never part of Client.dsp
	# (the EXE). The root-level copies here still #include the root mp3.h,
	# which pulls in the real <dsound.h> and redefines _DSBPOSITIONNOTIFY
	# against basic/AudioTypes.h's compat typedef (already processed earlier
	# in the TU via Client_PCH.h) - error C2011.
	list(FILTER CLIENT_MAIN_SOURCES EXCLUDE REGEX "/Client/reader\\.cpp$")
	list(FILTER CLIENT_MAIN_SOURCES EXCLUDE REGEX "/Client/synfilt\\.cpp$")
	list(FILTER CLIENT_MAIN_SOURCES EXCLUDE REGEX "/Client/subdecoder\\.cpp$")

	# Same duplication again: CSpritePal.cpp was SpriteLib.lib-only in the
	# original VC6 project (client-master_vs6/Client/SpriteLib/SpriteLib.dsp)
	# and never part of Client.dsp (the EXE). The root-level copy here was
	# compiling straight into DarkEden.exe too, fighting the SpriteLib.lib
	# copy for every CSpritePal symbol at link time (LNK2005).
	list(FILTER CLIENT_MAIN_SOURCES EXCLUDE REGEX "/Client/CSpritePal\\.cpp$")

	# Backup files (e.g. MItemTable_bak-2007-5-7.cpp) are stale duplicates of
	# their non-"_bak" counterpart and were never meant to compile on any
	# platform - this used to be scoped inside the `if(NOT WIN32)` block
	# below, so on Windows these still got globbed in and fought their
	# current counterparts for the same symbols at link time (LNK2005).
	list(FILTER CLIENT_MAIN_SOURCES EXCLUDE REGEX ".*\\.bak.*")
	list(FILTER CLIENT_MAIN_SOURCES EXCLUDE REGEX ".*_bak-.*")

	# Exclude Windows-specific files
	if(NOT WIN32)
		list(FILTER CLIENT_MAIN_SOURCES EXCLUDE REGEX ".*Imm.*")
		list(FILTER CLIENT_MAIN_SOURCES EXCLUDE REGEX ".*D3D.*")
		list(FILTER CLIENT_MAIN_SOURCES EXCLUDE REGEX ".*Direct3.*")
		# list(FILTER CLIENT_MAIN_SOURCES EXCLUDE REGEX ".*GameMain.*")  # Re-enabled for cross-platform support
		list(FILTER CLIENT_MAIN_SOURCES EXCLUDE REGEX ".*EXECryptor.*")
		list(FILTER CLIENT_MAIN_SOURCES EXCLUDE REGEX ".*GetWinVer.*")
		list(FILTER CLIENT_MAIN_SOURCES EXCLUDE REGEX ".*MInternetConnection.*")
		# Exclude Handler files in Client root only (not in Packet subdirectories)
		list(FILTER CLIENT_MAIN_SOURCES EXCLUDE REGEX "^Client/[^/]+Handler\\.cpp")
		# Exclude Client.cpp and ClientFunction.cpp - they're already in VS_UI library
		list(FILTER CLIENT_MAIN_SOURCES EXCLUDE REGEX "^Client/Client\\.cpp")
		list(FILTER CLIENT_MAIN_SOURCES EXCLUDE REGEX "^Client/ClientFunction\\.cpp")
		# MWorkThread and MMusic are needed on all platforms
		# list(FILTER CLIENT_MAIN_SOURCES EXCLUDE REGEX ".*MWorkThread.*")
		# list(FILTER CLIENT_MAIN_SOURCES EXCLUDE REGEX ".*MMusic\\.cpp")
		# Note: PacketFunction.cpp now included with proper platform guards
		# Windows-specific NetBIOS/IPX/SPX code is wrapped in #ifdef PLATFORM_WINDOWS
		# list(FILTER CLIENT_MAIN_SOURCES EXCLUDE REGEX ".*PacketFunction\\.cpp")
		# ProfileManager needed on all platforms
		# list(FILTER CLIENT_MAIN_SOURCES EXCLUDE REGEX ".*ProfileManager\\.cpp")
		list(FILTER CLIENT_MAIN_SOURCES EXCLUDE REGEX ".*VolumeOut.*\\.cpp")
		list(FILTER CLIENT_MAIN_SOURCES EXCLUDE REGEX ".*WavePackFileManager\\.cpp")
		# Exclude Windows-specific WinMain files
		list(FILTER CLIENT_MAIN_SOURCES EXCLUDE REGEX "WinLib/CWinMain\\.cpp")
		# Exclude Client.cpp on non-Windows (uses WinMain)
		list(FILTER CLIENT_MAIN_SOURCES EXCLUDE REGEX "Client/Client\\.cpp")
	endif()

	# Add DarkEden executable
	# WIN32 keyword required on Windows: Client/Client.cpp (compiled into the VS_UI
	# lib below) defines a real WinAPI WinMain, so the exe must link as a GUI
	# subsystem app (not console/main).
	if(WIN32)
		add_executable(DarkEden WIN32 ${CLIENT_MAIN_SOURCES})
	else()
		add_executable(DarkEden ${CLIENT_MAIN_SOURCES})
	endif()

	# Set output directory
	set_target_properties(DarkEden PROPERTIES
		RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
	)

	# atls.lib for VS_UI_WebBrowser.h's ATL usage - see the
	# _atlmfc_include_dir/_atlmfc_lib_dir search above (VS_UI target).
	if(_atlmfc_lib_dir)
		target_link_directories(DarkEden PRIVATE ${_atlmfc_lib_dir})
	endif()

	# Link libraries (now includes VS_UI and TextSystem)
	target_link_libraries(DarkEden PRIVATE
		SpriteLib
		dxlib
		basic
		framelib
		VS_UI
		TextSystem
		SDL2::SDL2
	)

	# Standard Windows import libraries for APIs this project calls
	# directly (Winsock: SocketAPI.cpp/DatagramSocket.cpp/...; Winmm:
	# WavePackFileManager.cpp's mmio* calls; WinInet:
	# MInternetConnection.cpp; Comctl32: InitCommonControls). None of
	# these were previously in the link line, so every symbol from them
	# was an unresolved external (LNK2001/LNK2019) once the code that
	# calls them started compiling.
	if(WIN32)
		target_link_libraries(DarkEden PRIVATE
			ws2_32
			winmm
			wininet
			comctl32
			netapi32
		)
	endif()

	# SDL2main provides its own WinMain that forwards to SDL_main() on Windows.
	# Client/Client.cpp defines WinMain directly (not via the SDL_main macro
	# convention), so linking SDL2main on Windows would collide with it
	# (LNK2005). It's a no-op on other platforms, so only link it there.
	if(NOT WIN32)
		target_link_libraries(DarkEden PRIVATE SDL2::SDL2main)
	endif()

	# Link iconv library (not needed for Emscripten - has stub implementations)
	if(NOT EMSCRIPTEN)
		if(Iconv_FOUND)
			target_link_libraries(DarkEden PRIVATE Iconv::Iconv)
		else()
			target_link_libraries(DarkEden PRIVATE iconv)
		endif()
	endif()

	# Link JPEG library if available
	if(JPEG_LIBRARIES)
		target_link_libraries(DarkEden PRIVATE ${JPEG_LIBRARIES})
		if(JPEG_INCLUDE_DIRS)
			target_include_directories(DarkEden PRIVATE ${JPEG_INCLUDE_DIRS})
		endif()
	endif()

	# Include directories
	target_include_directories(DarkEden PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}
		${CMAKE_CURRENT_SOURCE_DIR}/basic
		${CMAKE_CURRENT_SOURCE_DIR}/Client
		${CMAKE_CURRENT_SOURCE_DIR}/Client/TextSystem
		${CMAKE_CURRENT_SOURCE_DIR}/Client/DXLib
		${CMAKE_CURRENT_SOURCE_DIR}/Client/SpriteLib
		${CMAKE_CURRENT_SOURCE_DIR}/Client/Packet
		${CMAKE_CURRENT_SOURCE_DIR}/Client/framelib
#		${CMAKE_CURRENT_SOURCE_DIR}/tools/engine/sprite/include
#		${CMAKE_CURRENT_SOURCE_DIR}/tools/engine/ui/include
		${CMAKE_CURRENT_SOURCE_DIR}/VS_UI/src/header
		${CMAKE_CURRENT_SOURCE_DIR}/VS_UI/src/widget
		${CMAKE_CURRENT_SOURCE_DIR}/VS_UI/src/hangul
	)

	# Add Immersion library include path only on Windows
	if(WIN32)
		target_include_directories(DarkEden PRIVATE
			${CMAKE_CURRENT_SOURCE_DIR}/VS_UI/src/Imm
		)
	endif()

	# Compile definitions
	target_compile_definitions(DarkEden PRIVATE
		USE_SDL_BACKEND
		SPRITELIB_BACKEND_SDL
		__GAME_CLIENT__=1
	)

	# Platform-specific definitions
	if(NOT WIN32)
		target_compile_definitions(DarkEden PRIVATE PLATFORM_MACOS)
	else()
		# __WIN32__ was always passed via /D in the original VC6 build
		# (Client.dsp) and Client/Packet/Exception.h's __BEGIN_DEBUG/__END_DEBUG
		# (and Assert1.h's Assert()) macros are conditioned on it. Without it,
		# neither the NDEBUG nor the __WIN32__ branch matches on a Windows
		# Debug build, leaving those macros undefined across every Packet/*
		# translation unit that uses them.
		# __WINDOWS__ was likewise always passed via /D in Client.dsp.
		# Several Packet/*.cpp files (SocketAPI.cpp, PacketFileAPI.cpp, ...)
		# branch on bare `#if __WINDOWS__`/`#elif __WINDOWS__` (not
		# `defined(__WINDOWS__)`) to pick the Windows vs. POSIX code path;
		# an undefined macro reads as 0 in a #if, so without this those
		# branches silently compiled neither path, leaving locals like
		# fd/result/client/nSent declared in the branch that never ran.
		target_compile_definitions(DarkEden PRIVATE __WIN32__ __WINDOWS__)
	endif()

	message(STATUS "DarkEden executable will be built from remaining Client files (VS_UI compiled separately)")
endif()

# Print configuration summary
message(STATUS "")
message(STATUS "=== Dark Eden Client Configuration ===")
message(STATUS "  Build Tests:      ${BUILD_TESTS}")
message(STATUS "  Use SDL Backend:  ${USE_SDL_BACKEND}")
message(STATUS "  Build Engine:     ${BUILD_ENGINE}")
message(STATUS "  C++ Standard:     ${CMAKE_CXX_STANDARD}")
message(STATUS "=======================================")
message(STATUS "")
