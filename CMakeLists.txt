cmake_minimum_required(VERSION 3.16)
project(MathWizardAdventure CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Download and build raylib automatically (no manual install needed)
include(FetchContent)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG 5.5
)
FetchContent_MakeAvailable(raylib)

add_executable(MathWizard main.cpp)
target_link_libraries(MathWizard PRIVATE raylib)

# Copy the assets folder next to the executable after every build
add_custom_command(TARGET MathWizard POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:MathWizard>/assets)
