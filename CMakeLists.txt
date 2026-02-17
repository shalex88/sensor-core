cmake_minimum_required(VERSION 3.21)

set(PROJECT_NAME sensor-core)
include(cmake/SetVersion.cmake)
include(cmake/EnableVcpkg.cmake)

project(${PROJECT_NAME} LANGUAGES CXX VERSION ${VERSION})
add_compile_definitions(APP_NAME="${PROJECT_NAME}")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(SOURCE_DIR "src")
set(CONFIG_DIR "config")

# Use global include directories to provide access for the tests
include_directories(${PROJECT_NAME}
    ${SOURCE_DIR}
    ${proto_INCLUDE_DIRS}
)

file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS "${SOURCE_DIR}/*.cpp")

add_subdirectory(contracts/proto)
find_package(spdlog CONFIG REQUIRED)
find_package(CLI11 CONFIG REQUIRED)
find_package(yaml-cpp CONFIG REQUIRED)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

function(link_common_libraries target)
    target_link_libraries(${target}
        PRIVATE
        proto
        spdlog::spdlog_header_only
        yaml-cpp::yaml-cpp
        CLI11::CLI11
    )
endfunction(link_common_libraries)
link_common_libraries(${PROJECT_NAME})

include(GNUInstallDirs)
if(NOT DEFINED INSTALL_ROOT)
    set(INSTALL_ROOT "/tmp/" CACHE PATH "Root install dir for ${PROJECT_NAME}" FORCE)
endif()
set(CMAKE_INSTALL_PREFIX "${INSTALL_ROOT}" CACHE PATH "Install prefix" FORCE)
set(PROJECT_INSTALL_ROOT "${INSTALL_ROOT}/${PROJECT_NAME}" CACHE PATH "Root install dir for ${PROJECT_NAME}")
set(PROJECT_INSTALL_BINDIR "${PROJECT_INSTALL_ROOT}")
set(PROJECT_INSTALL_LIBDIR "${PROJECT_INSTALL_ROOT}/${CMAKE_INSTALL_LIBDIR}")
set(PROJECT_INSTALL_CONFIGDIR "${PROJECT_INSTALL_ROOT}/${CONFIG_DIR}")
set(CMAKE_INSTALL_RPATH "$ORIGIN/${CMAKE_INSTALL_LIBDIR}")
set(CMAKE_BUILD_RPATH  "$ORIGIN/${CMAKE_INSTALL_LIBDIR}")
set(CMAKE_SKIP_BUILD_RPATH OFF)
set(CMAKE_SKIP_INSTALL_RPATH OFF)
set(CMAKE_BUILD_WITH_INSTALL_RPATH OFF)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH ON)
set_target_properties(${PROJECT_NAME} PROPERTIES
    INSTALL_RPATH "${CMAKE_INSTALL_RPATH}"
    BUILD_RPATH   "${CMAKE_BUILD_RPATH}"
)

install(TARGETS ${PROJECT_NAME}
    COMPONENT runtime
    RUNTIME DESTINATION "${PROJECT_INSTALL_ROOT}"
)

install(DIRECTORY "${CMAKE_SOURCE_DIR}/config/"
    COMPONENT runtime
    DESTINATION "${PROJECT_INSTALL_CONFIGDIR}"
)

add_subdirectory(tests)
include(cmake/CreatePackage.cmake)
