project(${PROJECT_NAME})

enable_testing()

set(gtest_force_shared_crt OFF CACHE BOOL "Use shared (DLL) run-time lib even when Google Test is built as static lib" FORCE)
set(INSTALL_GTEST OFF CACHE BOOL "Enable installation of googletest" FORCE)
set(INSTALL_GMOCK OFF CACHE BOOL "Enable installation of googletest" FORCE)

include(FetchContent)
FetchContent_Declare(
        GoogleTest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.17.0
)

FetchContent_MakeAvailable(GoogleTest)

file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/${SOURCE_DIR}/*.cpp")
file(GLOB_RECURSE MAIN_CPP_FILES CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/${SOURCE_DIR}/*main.cpp")
foreach(MAIN_CPP ${MAIN_CPP_FILES})
    list(REMOVE_ITEM SOURCE_FILES "${MAIN_CPP}")
endforeach()

file(GLOB TEST_DIRS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *)
list(REMOVE_ITEM TEST_DIRS "CMakeLists.txt")

foreach(TEST_DIR ${TEST_DIRS})
    if(IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${TEST_DIR})
        set(TARGET_NAME ${PROJECT_NAME}-${TEST_DIR}-tests)
        file(GLOB_RECURSE TEST_FILES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/${TEST_DIR}/*.cpp")

        if(TEST_FILES)
            add_executable(${TARGET_NAME} ${TEST_FILES} ${SOURCE_FILES})
            target_link_libraries(${TARGET_NAME} PRIVATE gtest gtest_main gmock)
            link_common_libraries(${TARGET_NAME})

            include(GoogleTest)

            if(CMAKE_CROSSCOMPILING)
                gtest_add_tests(TARGET ${TARGET_NAME}
                        SOURCES ${TEST_FILES}
                        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
            else()
                gtest_discover_tests(${TARGET_NAME})
                target_compile_options(${TARGET_NAME} PRIVATE --coverage)
                target_link_options(${TARGET_NAME} PRIVATE --coverage)
            endif()

            message(STATUS "Created test target: ${TARGET_NAME}")
        endif()
    endif()
endforeach()