/* OmniOS — flutter-embedder/CMakeLists.txt */
/* Faz 1 Foundation */
/* SPDX-License-Identifier: MIT */

cmake_minimum_required(VERSION 3.28)
project(omnios-flutter-embedder VERSION 0.1.0 LANGUAGES C)

find_package(PkgConfig REQUIRED)
pkg_check_modules(FLUTTER_GLES REQUIRED glesv2)
pkg_check_modules(FLUTTER_EGL  REQUIRED egl)
pkg_check_modules(WAYLAND_CLIENT REQUIRED wayland-client)
pkg_check_modules(WAYLAND_EGL    REQUIRED wayland-egl)
pkg_check_modules(WAYLAND_PROTO  REQUIRED wayland-protocols)
pkg_check_modules(XKBCOMMON      REQUIRED xkbcommon)

find_library(FLUTTER_ENGINE_LIB
    NAMES flutter_engine flutter_engine_shared
    PATHS /usr/lib /usr/lib/omnios /usr/local/lib
)

if(NOT FLUTTER_ENGINE_LIB)
    message(FATAL_ERROR "Flutter engine library not found")
endif()

add_executable(omnios-flutter-embedder
    embedder.c
)

target_include_directories(omnios-flutter-embedder PRIVATE
    ${FLUTTER_GLES_INCLUDE_DIRS}
    ${FLUTTER_EGL_INCLUDE_DIRS}
    ${WAYLAND_CLIENT_INCLUDE_DIRS}
    ${WAYLAND_EGL_INCLUDE_DIRS}
    ${WAYLAND_PROTO_INCLUDE_DIRS}
    ${XKBCOMMON_INCLUDE_DIRS}
)

target_link_libraries(omnios-flutter-embedder PRIVATE
    ${FLUTTER_ENGINE_LIB}
    ${FLUTTER_GLES_LIBRARIES}
    ${FLUTTER_EGL_LIBRARIES}
    ${WAYLAND_CLIENT_LIBRARIES}
    ${WAYLAND_EGL_LIBRARIES}
    ${XKBCOMMON_LIBRARIES}
    m pthread dl
)

target_compile_options(omnios-flutter-embedder PRIVATE -Wall -Wextra -O2)

install(TARGETS omnios-flutter-embedder RUNTIME DESTINATION bin)
