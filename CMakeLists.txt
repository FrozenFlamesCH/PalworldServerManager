cmake_minimum_required(VERSION 3.20)
project(PalworldServerManager VERSION 1.0.0 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Options
option(BUILD_WITH_VULKAN "Build with Vulkan backend" OFF)
option(BUILD_WITH_OPENGL "Build with OpenGL backend" ON)

# Find packages
find_package(CURL REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(Threads REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

if(BUILD_WITH_OPENGL)
    find_package(OpenGL REQUIRED)
    find_package(glfw3 3.3 REQUIRED)
endif()

# SQLite3
add_library(sqlite3 STATIC third_party/sqlite3/sqlite3.c)
target_include_directories(sqlite3 PUBLIC third_party/sqlite3)

# ImGui
set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/third_party/imgui)
add_library(imgui STATIC
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)
target_include_directories(imgui PUBLIC ${IMGUI_DIR} ${IMGUI_DIR}/backends)
target_link_libraries(imgui PUBLIC glfw OpenGL::GL)

# stb
add_library(stb INTERFACE)
target_include_directories(stb INTERFACE third_party/stb)

# Main executable
file(GLOB_RECURSE SOURCES
    src/*.cpp
    src/*.h
)
add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/third_party
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    imgui
    sqlite3
    stb
    nlohmann_json::nlohmann_json
    CURL::libcurl
    OpenSSL::SSL
    OpenSSL::Crypto
    Threads::Threads
    fmt::fmt
    glfw
    OpenGL::GL
)

if(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE ws2_32 shlwapi)
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        _WIN32_WINNT=0x0601
        NOMINMAX
        WIN32_LEAN_AND_MEAN
    )
endif()

target_compile_definitions(${PROJECT_NAME} PRIVATE
    IMGUI_DEFINE_MATH_OPERATORS
    PSM_VERSION="${PROJECT_VERSION}"
)

# UTF-8 for MSVC (required by fmt)
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /utf-8)
endif()

# Install
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)
install(DIRECTORY config/ DESTINATION bin/config)
install(DIRECTORY assets/ DESTINATION bin/assets)