cmake_minimum_required(VERSION 3.24)
project(IcyTowerPortable C)

# Portable SDL3 source port of the frozen Icy Tower 1.5.1 reconstruction.
# See docs/port/ARCHITECTURE.md.  The historical matching toolchain under
# tools/ (verify.py, promote.py, link.py ...) belongs to the frozen commit and
# is not part of this build.

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_EXTENSIONS ON)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(FetchSDL3)

option(ITOWER_BUILD_TESTS "Build port unit tests" ON)

if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
  set(ITOWER_WARN -Wall -Wno-unused-variable -Wno-unused-but-set-variable
      -Wno-pointer-sign -Wno-unused-function -Wno-misleading-indentation
      -Wno-dangling-else -Wno-parentheses -Wno-format-overflow)
  # The historical sources rely on C89 inline semantics and permissive typing.
  set(ITOWER_GAME_FLAGS -fgnu89-inline -fno-strict-aliasing -fwrapv
      -Wno-implicit-function-declaration -Wno-int-conversion
      -Wno-incompatible-pointer-types -Wno-builtin-declaration-mismatch
      -Wno-format -Wno-char-subscripts -Wno-return-type -Wno-restrict
      -Wno-stringop-overflow -Wno-array-bounds -Wno-address)
endif()

# ---------------------------------------------------------------- xiph
# Ogg Vorbis decoding for the game's .ogg music/samples (same library
# versions as the historical build, vendored in third_party/).
set(XIPH_VORBIS ${CMAKE_CURRENT_SOURCE_DIR}/third_party/libvorbis-1.2.0)
set(XIPH_OGG ${CMAKE_CURRENT_SOURCE_DIR}/third_party/libogg-1.1.3)
add_library(itower_xiph STATIC
  ${XIPH_OGG}/src/framing.c ${XIPH_OGG}/src/bitwise.c
  ${XIPH_VORBIS}/lib/vorbisfile.c ${XIPH_VORBIS}/lib/info.c
  ${XIPH_VORBIS}/lib/block.c ${XIPH_VORBIS}/lib/synthesis.c
  ${XIPH_VORBIS}/lib/analysis.c ${XIPH_VORBIS}/lib/registry.c
  ${XIPH_VORBIS}/lib/sharedbook.c ${XIPH_VORBIS}/lib/psy.c
  ${XIPH_VORBIS}/lib/codebook.c ${XIPH_VORBIS}/lib/mdct.c
  ${XIPH_VORBIS}/lib/smallft.c ${XIPH_VORBIS}/lib/envelope.c
  ${XIPH_VORBIS}/lib/bitrate.c ${XIPH_VORBIS}/lib/lpc.c
  ${XIPH_VORBIS}/lib/window.c ${XIPH_VORBIS}/lib/floor0.c
  ${XIPH_VORBIS}/lib/floor1.c ${XIPH_VORBIS}/lib/res0.c
  ${XIPH_VORBIS}/lib/mapping0.c ${XIPH_VORBIS}/lib/lsp.c)
target_include_directories(itower_xiph PUBLIC ${XIPH_VORBIS}/include ${XIPH_OGG}/include
                           PRIVATE ${XIPH_VORBIS}/lib)
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
  target_compile_options(itower_xiph PRIVATE -w)
endif()
if(NOT WIN32)
  target_link_libraries(itower_xiph PUBLIC m)
endif()

# ----------------------------------------------------- compat + port core
# The Allegro 4 subset (compat/allegro4-sdl3) and the SDL3 platform, input,
# render and simulation subsystems (port/) form one static library because
# they reference each other (compat drives the platform service loop, the
# renderers read compat bitmaps).
file(GLOB A4_SOURCES CONFIGURE_DEPENDS
     ${CMAKE_CURRENT_SOURCE_DIR}/compat/allegro4-sdl3/src/*.c)
file(GLOB PORT_SOURCES CONFIGURE_DEPENDS
     ${CMAKE_CURRENT_SOURCE_DIR}/port/platform/*.c
     ${CMAKE_CURRENT_SOURCE_DIR}/port/input/*.c
     ${CMAKE_CURRENT_SOURCE_DIR}/port/render/*.c
     ${CMAKE_CURRENT_SOURCE_DIR}/port/sim/*.c
     ${CMAKE_CURRENT_SOURCE_DIR}/port/config/*.c)
add_library(itower_port STATIC ${A4_SOURCES} ${PORT_SOURCES})
target_include_directories(itower_port
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/compat/allegro4-sdl3/include
         ${CMAKE_CURRENT_SOURCE_DIR}
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/compat/allegro4-sdl3/src)
target_link_libraries(itower_port PUBLIC ${ITOWER_SDL3_TARGET} itower_xiph)
target_compile_options(itower_port PRIVATE ${ITOWER_WARN})
if(NOT WIN32)
  target_link_libraries(itower_port PUBLIC m)
endif()

# ------------------------------------------------------------- the game
# The 25 historical units, minus the dead advertising/network code which is
# replaced by port/game/ads_stub.c (see docs/port/ARCHITECTURE.md).
set(GAME_UNITS beta control custom directories game_data hisc loadpng main map
    menu options particle player profile regpng replay savepng scroller stars timer)
set(GAME_SOURCES)
foreach(u ${GAME_UNITS})
  list(APPEND GAME_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/${u}.c)
endforeach()
file(GLOB PORT_GAME_SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/port/game/*.c)
set(ITOWER_GAME_EXISTS TRUE)
foreach(f ${GAME_SOURCES})
  if(NOT EXISTS ${f})
    set(ITOWER_GAME_EXISTS FALSE)
  endif()
endforeach()
if(PORT_GAME_SOURCES)
  add_executable(icytower ${GAME_SOURCES} ${PORT_GAME_SOURCES})
  target_include_directories(icytower PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
  target_link_libraries(icytower PRIVATE itower_port)
  target_compile_options(icytower PRIVATE ${ITOWER_WARN} ${ITOWER_GAME_FLAGS})
  target_compile_definitions(icytower PRIVATE ICYTOWER_PORT=1)
  if(WIN32)
    set_target_properties(icytower PROPERTIES WIN32_EXECUTABLE TRUE)
  endif()
endif()

# ------------------------------------------------------------- tests
if(ITOWER_BUILD_TESTS)
  file(GLOB PORT_TEST_SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/tests/port/unit/*.c)
  foreach(t ${PORT_TEST_SOURCES})
    get_filename_component(tname ${t} NAME_WE)
    add_executable(${tname} ${t})
    target_link_libraries(${tname} PRIVATE itower_port)
    target_include_directories(${tname} PRIVATE
      ${CMAKE_CURRENT_SOURCE_DIR}/compat/allegro4-sdl3/src
      ${CMAKE_CURRENT_SOURCE_DIR}/tests/port)
  endforeach()
  file(GLOB GOLDEN_SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/tests/port/golden/*.c)
  list(FILTER GOLDEN_SOURCES EXCLUDE REGEX "main_allegro\.c$")
  add_executable(a4_golden_compat ${GOLDEN_SOURCES})
  target_link_libraries(a4_golden_compat PRIVATE itower_port)
  target_include_directories(a4_golden_compat PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/tests/port/golden)

  # the game's Ogg loader (port/game/logg_port.c) through the compat samples/mixer
  add_executable(test_logg ${CMAKE_CURRENT_SOURCE_DIR}/tests/port/logg/test_logg.c
                 ${CMAKE_CURRENT_SOURCE_DIR}/port/game/logg_port.c
                 ${CMAKE_CURRENT_SOURCE_DIR}/port/game/port_game.c)
  target_link_libraries(test_logg PRIVATE itower_port)
  target_include_directories(test_logg PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/compat/allegro4-sdl3/src)

  # Test scaffolding: until compat/allegro4-sdl3/src/a4_file.c provides
  # PACKFILE, executables that pull in a4_sound.c link a stdio stand-in.
  if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/compat/allegro4-sdl3/src/a4_file.c)
    foreach(t test_sound test_logg a4_golden_compat)
      if(TARGET ${t})
        target_sources(${t} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/tests/port/support/packfile_fallback.c)
      endif()
    endforeach()
  endif()
endif()
