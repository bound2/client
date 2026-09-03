cmake_minimum_required(VERSION 3.10)

# Basic library - Platform abstraction and utilities
# This is the lowest level library that all others depend on

project(basic)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Platform detection and library selection
if(WIN32)
	option(USE_SDL_BACKEND "Use SDL backend on Windows" OFF)

	if(USE_SDL_BACKEND)
		add_definitions(-DPLATFORM_USE_SDL)
		message(STATUS "Basic: Using SDL backend on Windows")
		# SDL2 should be found in parent scope
		if(NOT TARGET SDL2::SDL2)
			find_package(SDL2 REQUIRED)
		endif()
	else()
		message(STATUS "Basic: Using native Windows backend")
	endif()
else()
	# Force SDL on non-Windows platforms
	add_definitions(-DPLATFORM_USE_SDL)
	# SDL2 should be found in parent scope
	if(NOT TARGET SDL2::SDL2)
		find_package(SDL2 REQUIRED)
	endif()
	message(STATUS "Basic: Using SDL backend (required on this platform)")
endif()

# Basic library sources
set(BASIC_SOURCES
	Directory.cpp
	BasicData.cpp
	Timer2.cpp
	BasicException.cpp
	ColorDraw.cpp
	# The logging facility (docs/RESTRUCTURING.md task 5.1). It lived in
	# Client/ and was a gamemodel member, which put it above packetwire
	# in the link order and out of reach of the wire layer's include
	# rules. It needs nothing but the C library and Platform.h, so the
	# bottom library is where it belongs: every target links basic, and
	# this directory is a PUBLIC include directory, so #include
	# "DebugLog.h" resolves everywhere it did before.
	DebugLog.cpp
	# The checked formatter (docs/RESTRUCTURING.md task 5.4). It is here
	# for the same reason DebugLog is: the call sites that need it are in
	# the executable, in VS_UI and in the packet handlers, and basic is
	# the one library all of them already link. It needs nothing but the
	# C library.
	SafeFormat.cpp
)

# Platform-specific sources
# Windows native implementation removed (SDL2 migration) - Always use SDL backend
list(APPEND BASIC_SOURCES PlatformSDL.cpp)
message(STATUS "Basic: Compiling PlatformSDL.cpp")

# Create static library
add_library(basic STATIC ${BASIC_SOURCES})

# Public include directories
target_include_directories(basic
	PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}
)

# Link SDL2 if using SDL backend
if(USE_SDL_BACKEND OR NOT WIN32)
	if(TARGET SDL2::SDL2)
		target_link_libraries(basic PUBLIC SDL2::SDL2)
	else()
		target_link_libraries(basic PUBLIC ${SDL2_LIBRARIES})
		if(SDL2_INCLUDE_DIRS)
			target_include_directories(basic PUBLIC ${SDL2_INCLUDE_DIRS})
		endif()
	endif()
endif()

# Platform-specific link libraries
if(WIN32 AND NOT USE_SDL_BACKEND)
	# Windows native libraries
	target_link_libraries(basic PUBLIC
		winmm
	)

	# Disable specific warnings for MSVC
	if(MSVC)
		target_compile_options(basic PRIVATE /W3)
	endif()
endif()

# Compiler flags
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
	target_compile_options(basic PRIVATE
		-Wall
		-Wno-unknown-pragmas
	)
endif()

# Installation
install(TARGETS basic
	ARCHIVE DESTINATION lib
	LIBRARY DESTINATION lib
)

install(FILES
	Platform.h
	Typedef.h
	PlatformUtil.h
	Directory.h
	BasicMemory.h
	BasicException.h
	DLL.h
	timer2.h
	ColorDraw.h
	InputCodes.h
	AudioTypes.h
	SafeFormat.h
	DESTINATION include/basic
)
