cmake_minimum_required(VERSION 3.5)
project(raylib_httplib_json C CXX)

set(CMAKE_CXX_STANDARD 20)

# Setting parameters for raylib
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE) # don't build the supplied examples
set(BUILD_GAMES    OFF CACHE BOOL "" FORCE) # or games

include_directories(
        ${CMAKE_SOURCE_DIR}/external
        ${CMAKE_SOURCE_DIR}/src
)

add_subdirectory(libs/raylib)

add_executable(${PROJECT_NAME} src/main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE raylib)
target_compile_definitions(${PROJECT_NAME} PUBLIC ASSETS_PATH="${CMAKE_CURRENT_SOURCE_DIR}/assets/") # Set the asset path macro to the absolute path on the dev machine

if (WIN32)
    target_compile_definitions(${PROJECT_NAME} PRIVATE
            WIN32_LEAN_AND_MEAN
            NOMINMAX
            NOGDI
            NOUSER
            _WIN32_WINNT=0x0A00
    )

    target_link_libraries(${PROJECT_NAME} PRIVATE ws2_32)
endif()