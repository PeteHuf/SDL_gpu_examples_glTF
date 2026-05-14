cmake_minimum_required(VERSION 3.10)
project(SDL_gpu_examples)

option(ENABLE_DX_DEBUGGING "Enables enhaced debugging with DX12 backend" OFF)

set(CMAKE_CXX_STANDARD 23) # PRECHECKIN: should be in CMakePresets.json file?

find_package(SDL3 CONFIG QUIET)

## treat .c as .cpp
#file(GLOB_RECURSE C_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.c") # PRECHECKIN: remove, too annoying to port all to cpp?
#set_source_files_properties(${C_FILES} PROPERTIES LANGUAGE CXX)


if ((APPLE AND NOT CMAKE_SYSTEM_NAME MATCHES "Darwin") OR EMSCRIPTEN)
    set(BUILD_SHARED_LIBS OFF CACHE INTERNAL "")    # Disable shared builds on platforms where it does not make sense to use them
    set(SDL_SHARED OFF)
else()
    set(SDL_SHARED ON)
endif()

if(MSVC)
    if(NOT CMAKE_GENERATOR STREQUAL "Ninja")
        add_definitions(/MP)				# parallelize each target, unless Ninja is the generator
    endif()
endif()

if(MSVC)
    set(xpl_compile_options "/Zc:__cplusplus" "/bigobj") # PRECHECKIN: should be in CMakePresets.json file?
    set(xpl_compile_definitions "_UNICODE" "UNICODE")
else()
    set(xpl_compile_options)
    set(xpl_compile_definitions)
endif()

if (ENABLE_DX_DEBUGGING)
    add_definitions(-DSDL_GLTF_DX_DEBUGGING)
endif()

#if(NOT SDL3_FOUND)
#    message(STATUS "SDL3 not found. Building it from source.")
#    include(FetchContent)
#    FetchContent_Declare(
#            SDL
#            GIT_REPOSITORY https://github.com/libsdl-org/SDL
#            GIT_TAG        "release-3.4.8"
#    )
#    FetchContent_MakeAvailable(SDL)
#else()
#    find_package(SDL3 REQUIRED)
#endif()


# Configure SDL by calling its CMake file.
# we use EXCLUDE_FROM_ALL so that its install targets and configs don't
# pollute upwards into our configuration.
add_subdirectory(External/SDL EXCLUDE_FROM_ALL)


#add_subdirectory(External/SDL)
include_directories(External/glm)
include_directories(External/tinygltf)

# Set the name of the executable
set(EXECUTABLE_NAME SDL_gpu_examples)

# Create an executable or a shared library based on the platform and add our sources to it
if (ANDROID)
    set(ANDROID_SUPPORT_FLEXIBLE_PAGE_SIZES ON)

    # The SDL java code is hardcoded to load libmain.so on android, so we need to change EXECUTABLE_NAME
    set(EXECUTABLE_NAME main)
    add_library(${EXECUTABLE_NAME} SHARED)
else()
    add_executable(${EXECUTABLE_NAME})
endif()

# Add your sources to the target
target_sources(${EXECUTABLE_NAME}
        PRIVATE
        Examples/main.c
        Examples/Common.h
        stb_image.h
        Examples/Common.c
        Examples/ClearScreen.c
        Examples/ClearScreenMultiWindow.c
        Examples/BasicTriangle.c
        Examples/BasicVertexBuffer.c
        Examples/CullMode.c
        Examples/BasicStencil.c
        Examples/InstancedIndexed.c
        Examples/TexturedQuad.c
        Examples/ComputeSampler.c
        Examples/TexturedAnimatedQuad.c
        Examples/Clear3DSlice.c
        Examples/BasicCompute.c
        Examples/ComputeUniforms.c
        Examples/ToneMapping.c
        Examples/CustomSampling.c
        Examples/DrawIndirect.c
        Examples/ComputeSpriteBatch.c
        Examples/CopyAndReadback.c
        Examples/CopyConsistency.c
        Examples/Texture2DArray.c
        Examples/TriangleMSAA.c
        Examples/Cubemap.c
        Examples/WindowResize.c
        Examples/Blit2DArray.c
        Examples/BlitCube.c
        Examples/BlitMirror.c
        Examples/GenerateMipmaps.c
        Examples/GlTF.cpp
        Examples/Latency.c
        Examples/DepthSampler.c
        Examples/PullSpriteBatch.c
        Examples/TextureTypeTest.c
        Examples/CompressedTextures.c
        Src/glTFModel.cpp
        Src/glTFModel.h
        Src/VulkanDevice.hpp
)

target_link_libraries(${EXECUTABLE_NAME}
    SDL3::SDL3
    $<$<BOOL:${ENABLE_DX_DEBUGGING}>:dxgi.lib>
)

target_include_directories(${EXECUTABLE_NAME}
    PRIVATE
    Src
)

if (ANDROID)
     if (NOT MOBILE_ASSETS_DIR)
        message(FATAL_ERROR "Building for Android, but MOBILE_ASSETS_DIR is not set")
    endif()
    file(MAKE_DIRECTORY "${MOBILE_ASSETS_DIR}")    # we need to create the project Assets dir otherwise CMake won't copy our assets.

    # MOBILE_ASSETS_DIR is set in the gradle file via the cmake command line and points to the Android Studio Assets folder. 
    # when we copy assets there, the Android build pipeline knows to add them to the apk. 
    #set(outname "${MOBILE_ASSETS_DIR}/${filename}")    

     # PRECHECKIN: using TARGET ${EXECUTABLE_NAME} wrong? copied from below but EXECUTABLE_NAME not referenced here
    add_custom_command(TARGET ${EXECUTABLE_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/Content ${MOBILE_ASSETS_DIR}/Content
    )
else()
    add_custom_command(TARGET ${EXECUTABLE_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/Content $<TARGET_FILE_DIR:${EXECUTABLE_NAME}>/Content
    )
endif()


if(WIN32)
    # copy the SDL3.dll to the target dir
    add_custom_command(
        TARGET ${EXECUTABLE_NAME} POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy $<TARGET_FILE:SDL3::SDL3-shared> $<TARGET_FILE_DIR:${EXECUTABLE_NAME}>
        VERBATIM
    )
endif(WIN32)
