# SDL3 dependency resolution for the portable build.
#
# 1. ITOWER_SDL3_MODE=system : find_package(SDL3 CONFIG REQUIRED)
# 2. ITOWER_SDL3_MODE=fetch  : download the pinned release source and build it
#                              statically as part of this project (default).
# The tarball is pinned by SHA-256; set ITOWER_DEPS_DIR to reuse a local cache.
set(ITOWER_SDL3_MODE "fetch" CACHE STRING "How to obtain SDL3: fetch or system")
set_property(CACHE ITOWER_SDL3_MODE PROPERTY STRINGS fetch system)
set(ITOWER_SDL3_VERSION "3.4.16")
set(ITOWER_SDL3_SHA256 "7322236cd12090c3eb40b9728be4d49c76f66ad17d04369584d4ecad5cf77c68")
set(ITOWER_DEPS_DIR "${CMAKE_SOURCE_DIR}/build/deps" CACHE PATH "Download cache for pinned dependencies")

if(ITOWER_SDL3_MODE STREQUAL "system")
  find_package(SDL3 CONFIG REQUIRED)
  set(ITOWER_SDL3_TARGET SDL3::SDL3)
else()
  include(FetchContent)
  set(_sdl_tar "${ITOWER_DEPS_DIR}/SDL3-${ITOWER_SDL3_VERSION}.tar.gz")
  if(NOT EXISTS "${_sdl_tar}")
    file(DOWNLOAD
      "https://github.com/libsdl-org/SDL/releases/download/release-${ITOWER_SDL3_VERSION}/SDL3-${ITOWER_SDL3_VERSION}.tar.gz"
      "${_sdl_tar}" EXPECTED_HASH SHA256=${ITOWER_SDL3_SHA256} SHOW_PROGRESS)
  endif()
  file(SHA256 "${_sdl_tar}" _sdl_hash)
  if(NOT _sdl_hash STREQUAL ITOWER_SDL3_SHA256)
    message(FATAL_ERROR "SDL3 tarball hash mismatch: ${_sdl_tar}")
  endif()
  set(SDL_SHARED OFF CACHE BOOL "" FORCE)
  set(SDL_STATIC ON CACHE BOOL "" FORCE)
  set(SDL_TEST_LIBRARY OFF CACHE BOOL "" FORCE)
  set(SDL_TESTS OFF CACHE BOOL "" FORCE)
  set(SDL_EXAMPLES OFF CACHE BOOL "" FORCE)
  set(SDL_INSTALL OFF CACHE BOOL "" FORCE)
  FetchContent_Declare(SDL3 URL "${_sdl_tar}" URL_HASH SHA256=${ITOWER_SDL3_SHA256}
                       DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
  FetchContent_MakeAvailable(SDL3)
  set(ITOWER_SDL3_TARGET SDL3::SDL3-static)
endif()
