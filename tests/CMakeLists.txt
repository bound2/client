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

add_executable(unit_tests
	framework/test_main.cpp
	${UNIT_TEST_SOURCES}
)

target_link_libraries(unit_tests PRIVATE
	test_framework
	basic
)

target_include_directories(unit_tests PRIVATE
	${CMAKE_SOURCE_DIR}
	${CMAKE_SOURCE_DIR}/basic
	${CMAKE_SOURCE_DIR}/Client/framelib
)

add_test(NAME unit_tests COMMAND unit_tests)
