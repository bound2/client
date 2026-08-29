cmake_minimum_required(VERSION 3.10)

# Sprite engine static library
# Exposes SP.h as the public API header

find_package(SDL2 REQUIRED)

add_library(sprite STATIC
    src/animation.c
    src/color.c
    src/colorset.c
    src/frame.c
    src/framepack.c
    src/index_sprite.c
    src/index_spritepack.c
    src/sdl_framework.c
    src/shadow_sprite.c
    src/shadow_spritepack.c
    src/sprite.c
    src/spritepack.c
    src/ui_surface.c
    src/zone.c
)

# Public includes
target_include_directories(sprite
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Link against SDL2
if(TARGET SDL2::SDL2)
    target_link_libraries(sprite PUBLIC SDL2::SDL2)
else()
    target_link_libraries(sprite PUBLIC ${SDL2_LIBRARIES})
    target_include_directories(sprite PUBLIC ${SDL2_INCLUDE_DIRS})
endif()

set_property(TARGET sprite PROPERTY POSITION_INDEPENDENT_CODE ON)

# The generic container macros in include/vector.h and include/map.h are built
# on typeof, which was a GNU extension until C23 standardised it. MSVC only
# accepts it in C23 mode, so without this the whole library fails to compile
# under Visual Studio ("'typeof' undefined; assuming extern returning int",
# followed by a cascade of syntax errors). GCC and Clang accept it as-is.
if(MSVC)
    target_compile_options(sprite PRIVATE /std:clatest)
endif()

# Install
install(TARGETS sprite
    ARCHIVE DESTINATION lib
)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/include/SP.h
    DESTINATION include/sprite
)
