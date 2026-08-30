cmake_minimum_required(VERSION 3.10)

#----------------------------------------------------------------------
# Unit tests
#----------------------------------------------------------------------
#
# Only code compiled into a static library can be linked into a test
# binary. Today that means basic/, framelib/ (header-only templates),
# SpriteLib/, DXLib/, TextSystem/ and VS_UI. Game logic that is compiled
# straight into the DarkEden executable is not reachable from here.
#
#----------------------------------------------------------------------

add_library(test_framework STATIC
	framework/test_framework.cpp
)

target_include_directories(test_framework PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/framework
)

#----------------------------------------------------------------------
# Test sources are globbed so adding a test only means adding a file to
# unit/, matching how the rest of this project collects sources.
#----------------------------------------------------------------------
file(GLOB UNIT_TEST_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/unit/*.cpp)

#----------------------------------------------------------------------
# stubs/ defines globals that a linked library references but that are
# defined in the DarkEden executable. See the file for why each one is
# there; they are not a general escape hatch for pulling game logic in.
#----------------------------------------------------------------------
add_executable(unit_tests
	framework/test_main.cpp
	stubs/client_globals.cpp
	${UNIT_TEST_SOURCES}
)

target_link_libraries(unit_tests PRIVATE
	test_framework
	basic
	SpriteLib
	TextSystem
)

target_include_directories(unit_tests PRIVATE
	${CMAKE_SOURCE_DIR}
	${CMAKE_SOURCE_DIR}/basic
	${CMAKE_SOURCE_DIR}/Client/framelib
	${CMAKE_SOURCE_DIR}/Client/TextSystem
)

#----------------------------------------------------------------------
# Wire-layout inventory (unit/test_wire_layout.cpp).
#
# That one source compiles every packet header under Client/Packet the
# way the DarkEden target does (__GAME_CLIENT__=1) but links none of the
# packet .cpp files: tests/generated/WireInventory.inc carries the
# factories' getPacketID/getPacketMaxSize bodies as plain functions.
# See tests/tools/gen_wire_inventory.pl.
#----------------------------------------------------------------------
target_sources(unit_tests PRIVATE stubs/wire_layout_defs.cpp)
set_source_files_properties(
	${CMAKE_CURRENT_SOURCE_DIR}/unit/test_wire_layout.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/stubs/wire_layout_defs.cpp
	PROPERTIES
		INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}/Client/Packet;${CMAKE_CURRENT_SOURCE_DIR}/generated"
		COMPILE_DEFINITIONS "__GAME_CLIENT__=1;WIRE_LAYOUT_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/wire-layout.txt\""
)

add_test(NAME unit_tests COMMAND unit_tests)

#----------------------------------------------------------------------
# The committed WireInventory.inc must match a fresh generation, so a
# new or changed factory cannot go uninventoried. Needs perl (Git for
# Windows ships one); without it the check is skipped, not passed.
#----------------------------------------------------------------------
find_program(PERL_EXECUTABLE perl)
if(PERL_EXECUTABLE)
	add_test(
		NAME wire_inventory_fresh
		COMMAND ${CMAKE_COMMAND}
			-DPERL_EXECUTABLE=${PERL_EXECUTABLE}
			-DSOURCE_DIR=${CMAKE_SOURCE_DIR}
			-P ${CMAKE_CURRENT_SOURCE_DIR}/tools/check_wire_inventory_fresh.cmake
	)
else()
	message(STATUS "perl not found: the wire_inventory_fresh test is not registered")
endif()
