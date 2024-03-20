cmake_minimum_required(VERSION 3.24)
project(opengl-renderer)

# Global settings
set(PREFER_EMBED_MODELS OFF CACHE BOOL "Prefer embedding models into executable" FORCE)
set(BUILD_SHARED_LIBS   ON  CACHE BOOL "Build shared instead of static libraries" FORCE)
set(CMAKE_CXX_FLAGS_RELEASE "/MT")
# Isolated settings
list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
include(SDL3Settings)
include(AssimpSettings)
include(GLbindingSettings)
include(MiscSettings)

include(FetchContent)
set(FETCHCONTENT_QUIET OFF) # enable git output for FetchContent steps
FetchContent_Declare(sdl3 GIT_REPOSITORY "https://github.com/libsdl-org/SDL.git" GIT_TAG "495e432fb9776fdc13a8d96179136064ccddf6e5")
#FetchContent_Declare(sdl3mixer GIT_REPOSITORY "https://github.com/libsdl-org/SDL_mixer.git" GIT_TAG "main")
FetchContent_Declare(assimp GIT_REPOSITORY "https://github.com/assimp/assimp.git" GIT_TAG "master")
FetchContent_Declare(glm GIT_REPOSITORY "https://github.com/g-truc/glm.git" GIT_TAG "bf71a834948186f4097caa076cd2663c69a10e1e")
FetchContent_Declare(stb GIT_REPOSITORY "https://github.com/M2-TE/stb_image.git" GIT_TAG "master")
FetchContent_Declare(cmrc GIT_REPOSITORY "https://github.com/vector-of-bool/cmrc.git" GIT_TAG "master")
FetchContent_Declare(glbinding GIT_REPOSITORY "https://github.com/cginternals/glbinding.git" GIT_TAG "master")
FetchContent_Declare(imgui GIT_REPOSITORY "https://github.com/ocornut/imgui.git" GIT_TAG "docking")
FetchContent_Declare(sponza GIT_REPOSITORY "https://github.com/jimmiebergmann/Sponza.git" GIT_TAG "master" SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/models/sponza")
FetchContent_MakeAvailable(sdl3 assimp glm stb cmrc glbinding imgui sponza)
set(ENABLE_ALL_WARNINGS OFF)
FetchContent_Declare(jolt GIT_REPOSITORY "https://github.com/jrouwe/JoltPhysics.git" GIT_TAG "master" SOURCE_SUBDIR "build")
FetchContent_MakeAvailable(jolt)
set(FETCHCONTENT_FULLY_DISCONNECTED ON CACHE BOOL "Faster config after FetchContent has run once" FORCE)
find_package(OpenGL REQUIRED COMPONENTS)

# embedding of cmrc resources
file(GLOB_RECURSE shader-files RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" CONFIGURE_DEPENDS "shaders/*")
cmrc_add_resource_library(shaders "${shader-files}") # embed shaders
file(GLOB_RECURSE image-files RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" CONFIGURE_DEPENDS "images/*")
cmrc_add_resource_library(images "${image-files}") # embed images
file(GLOB_RECURSE audio-files RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" CONFIGURE_DEPENDS "audio/*")
cmrc_add_resource_library(audio "${audio-files}") # embed audio

# copy audio data next to executable (if no cmrc embed is desired)
# file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/audio/" DESTINATION "${CMAKE_CURRENT_SOURCE_DIR}/bin/audio/")

# create executable target
file(GLOB_RECURSE source-files CONFIGURE_DEPENDS "src/*")
add_executable(${PROJECT_NAME} "${source-files}")
target_include_directories(${PROJECT_NAME} PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/include/"
    "${imgui_SOURCE_DIR}/"
    "${imgui_SOURCE_DIR}/backends")
file(GLOB imgui-sources CONFIGURE_DEPENDS "${imgui_SOURCE_DIR}/*.cpp")
target_sources(${PROJECT_NAME} PRIVATE
    "${imgui-sources}"
    "${imgui_SOURCE_DIR}/backends/imgui_impl_sdl3.cpp"
    "${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp")

# link libraries to executable
target_link_libraries(${PROJECT_NAME} glm::glm) # OpenGL math library
target_link_libraries(${PROJECT_NAME} SDL3::SDL3)
target_link_libraries(${PROJECT_NAME} Jolt)
#target_link_libraries(${PROJECT_NAME} SDL3_mixer::SDL3_mixer)
target_link_libraries(${PROJECT_NAME} OpenGL::GL) # OpenGL headers
target_link_libraries(${PROJECT_NAME} glbinding::glbinding) # OpenGL function loader
target_link_libraries(${PROJECT_NAME} assimp stb-image) # model/image loading
target_link_libraries(${PROJECT_NAME} shaders images audio) # CMRC embedded files

# only embed models with CMRC if desired
if (${PREFER_EMBED_MODELS})
    file(GLOB_RECURSE model-files RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" CONFIGURE_DEPENDS "models/*")
    cmrc_add_resource_library(models "${model-files}") # embed models into library "models"
    target_link_libraries(${PROJECT_NAME} models)
    target_compile_definitions(${PROJECT_NAME} PRIVATE EMBEDDED_MODELS=1)
endif()