cmake_minimum_required(VERSION 3.21)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  message(WARNING "CMAKE_TOOLCHAIN_FILE not set. If you use vcpkg, configure with the vcpkg toolchain (via CMakePresets is recommended).")
endif()

project(s100-sampler VERSION 0.95 LANGUAGES CXX)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY AUTOGEN_SOURCE_GROUP "Generated Files")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Dependencies
find_package(Qt5 REQUIRED COMPONENTS Widgets Svg)

# Prefer PATH; also hint vcpkg bin if available
find_program(XM_LLINT_EXECUTABLE xmllint
  HINTS "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/bin"
)
if (NOT XM_LLINT_EXECUTABLE)
  message(FATAL_ERROR "xmllint not found. Install libxml2/xmllint (via system packages or vcpkg) and ensure it's on PATH.")
endif()

set(CMAKE_USE_OPENSSL ON CACHE BOOL "Enable OpenSSL for HTTPS support" FORCE)
set(BUILD_CURL_EXE OFF CACHE BOOL "Disable curl executable" FORCE)

find_package(OpenSSL REQUIRED)
find_package(CURL REQUIRED)
find_package(SQLiteCpp CONFIG REQUIRED)
find_package(pugixml CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

# Qt autogen
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_AUTOUIC_SEARCH_PATHS
  "${CMAKE_CURRENT_SOURCE_DIR}/include/mainUI"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/MSR"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/S100"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/mainUI"
)

# Sources
file(GLOB FILES_INC
  "${CMAKE_CURRENT_SOURCE_DIR}/include/mainUI/*.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp"
)

file(GLOB FILES_UI
  "${CMAKE_CURRENT_SOURCE_DIR}/include/mainUI/*.ui"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/MSR/*.ui"
)

file(GLOB SOURCES
  "${CMAKE_CURRENT_SOURCE_DIR}/src/parser/*.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/mainUI/*.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/mainUI/*.ui"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/enumerations/*.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/S100/*.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/S100/*.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/S100_CodeLists/*.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/S124/*.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/S125/*.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/S212/*.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/basicAttributes/*.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/MSR/*.cpp"
)

# Windows resource file only on Windows
set(APP_RC "")
if(WIN32)
  set(APP_RC "${CMAKE_CURRENT_SOURCE_DIR}/resources/app.rc")
endif()

add_executable(${PROJECT_NAME}
  ${SOURCES}
  ${FILES_INC}
  ${FILES_UI}
  ${APP_RC}
)

target_include_directories(${PROJECT_NAME} PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/include"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/mainUI"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/mainUI"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/parser"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/enumerations"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/S100_CodeLists"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/S100"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/S124"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/S125"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/S212"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/NMEA"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/MSR"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/basicAttributes"
)

target_link_libraries(${PROJECT_NAME} PRIVATE
  SQLiteCpp
  OpenSSL::SSL OpenSSL::Crypto CURL::libcurl
  Qt5::Widgets Qt5::Svg
  pugixml::pugixml
  nlohmann_json::nlohmann_json
)

find_path(TOMLPLUSPLUS_INCLUDE_DIR NAMES toml++/toml.h toml++/toml.hpp REQUIRED)
find_path(CPPCODEC_INCLUDE_DIRS NAMES cppcodec/base64_rfc4648.hpp REQUIRED)

target_include_directories(${PROJECT_NAME} PRIVATE
  "${TOMLPLUSPLUS_INCLUDE_DIR}"
  "${CPPCODEC_INCLUDE_DIRS}"
)

# Default build type for single-config generators
if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "Build type" FORCE)
endif()

# Windows-specific defines
if(MSVC)
  target_compile_definitions(${PROJECT_NAME} PRIVATE
    NOMINMAX
    WIN32_LEAN_AND_MEAN
  )
endif()

# IMPORTANT: do NOT leak this onto Linux
if(WIN32)
  target_compile_definitions(${PROJECT_NAME} PRIVATE _WIN32_WINNT=0x0A00)
endif()

# Release flags (kept as you had them)
if (MSVC)
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2 /GL /fp:fast /arch:AVX2 /Gw /Gy /Qpar /EHsc /w34265")
  set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /LTCG /OPT:REF /OPT:ICF")
elseif (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -march=native -flto -ffast-math -fdata-sections -ffunction-sections -DNDEBUG")
  set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -Wl,--gc-sections")
endif()

message(STATUS "CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")
message(STATUS "CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")
message(STATUS "CMAKE_CXX_FLAGS_DEBUG: ${CMAKE_CXX_FLAGS_DEBUG}")
message(STATUS "CMAKE_CXX_FLAGS_RELEASE: ${CMAKE_CXX_FLAGS_RELEASE}")
message(STATUS "CMAKE_CXX_FLAGS_RELWITHDEBINFO: ${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")
message(STATUS "CMAKE_CXX_FLAGS_MINSIZEREL: ${CMAKE_CXX_FLAGS_MINSIZEREL}")

if(IDE_FOLDER)
  set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "${IDE_FOLDER}")
endif()

# -------------------------------
# CPack Packaging
# -------------------------------
include(InstallRequiredSystemLibraries)

set(CPACK_PACKAGE_NAME "${PROJECT_NAME}")
set(CPACK_PACKAGE_VENDOR "Novia")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "NOVIAECS Application")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "${PROJECT_NAME}")
set(CPACK_PACKAGE_EXECUTABLES "${PROJECT_NAME}" "${PROJECT_NAME}")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE.txt")
set(CPACK_RESOURCE_FILE_README "${CMAKE_SOURCE_DIR}/README.txt")

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)

# Use windeployqt for Qt runtime deployment on Windows installs
if (WIN32)
  find_program(WINDEPLOYQT_EXECUTABLE windeployqt
    HINTS "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/tools/qt5/bin"
    NO_DEFAULT_PATH
  )
  if (NOT WINDEPLOYQT_EXECUTABLE)
    message(FATAL_ERROR "windeployqt not found (need qt5-tools).")
  endif()

  install(CODE "
    set(_exe \"\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/bin/${PROJECT_NAME}.exe\")
    if (NOT EXISTS \"\${_exe}\")
      message(FATAL_ERROR \"Installed exe not found: \${_exe}\")
    endif()

    execute_process(
      COMMAND \"${WINDEPLOYQT_EXECUTABLE}\"
              --no-translations
              --no-system-d3d-compiler
              --no-angle
              \"\${_exe}\"
      RESULT_VARIABLE r
    )
    if (NOT r EQUAL 0)
      message(FATAL_ERROR \"windeployqt failed with code \${r}\")
    endif()
  ")
endif()

install(DIRECTORY "${CMAKE_SOURCE_DIR}/resources/"
  DESTINATION resources
  FILES_MATCHING PATTERN "*.*"
)

install(FILES "${CMAKE_SOURCE_DIR}/config.toml" DESTINATION .)

# Copy vcpkg runtime DLLs into installer (Windows + vcpkg only)
if (WIN32 AND DEFINED VCPKG_INSTALLED_DIR AND DEFINED VCPKG_TARGET_TRIPLET)
  install(DIRECTORY "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/bin/"
    DESTINATION bin
    FILES_MATCHING PATTERN "*.dll"
  )
endif()

if(WIN32)
  set(CPACK_GENERATOR "ZIP")
  set(CPACK_NSIS_DISPLAY_NAME "${PROJECT_NAME}")
  set(CPACK_NSIS_MODIFY_PATH ON)
endif()

include(CPack)
