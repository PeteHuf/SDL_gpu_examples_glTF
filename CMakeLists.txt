cmake_minimum_required(VERSION 3.10)
project(SDL_gpu_examples)

set(CMAKE_CXX_STANDARD 23) # PRECHECKIN: should be in CMakePresets.json file?

find_package(SDL3 CONFIG QUIET)

## treat .c as .cpp
#file(GLOB_RECURSE C_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.c") # PRECHECKIN: remove, too annoying to port all to cpp?
#set_source_files_properties(${C_FILES} PROPERTIES LANGUAGE CXX)


if(MSVC)
    set(xpl_compile_options "/Zc:__cplusplus" "/bigobj") # PRECHECKIN: should be in CMakePresets.json file?
    set(xpl_compile_definitions "_UNICODE" "UNICODE")
else()
    set(xpl_compile_options)
    set(xpl_compile_definitions)
endif()

add_subdirectory(External/SDL)

#if(NOT SDL3_FOUND)
#    message(STATUS "SDL3 not found. Building it from source.") # PRECHECKIN: remove
#    include(FetchContent)
#    FetchContent_Declare(
#            SDL
#            GIT_REPOSITORY https://github.com/libsdl-org/SDL
#            GIT_TAG        "release-3.2.16"
#    )
#    FetchContent_MakeAvailable(SDL)
#else()
#    find_package(SDL3 REQUIRED)
#endif()

add_executable(SDL_gpu_examples
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
)

target_link_libraries(SDL_gpu_examples
    SDL3::SDL3
)

add_custom_command(TARGET SDL_gpu_examples POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/Content $<TARGET_FILE_DIR:SDL_gpu_examples>/Content
)

if(WIN32)
    # copy the SDL3.dll to the target dir
    add_custom_command(
        TARGET SDL_gpu_examples POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy $<TARGET_FILE:SDL3::SDL3-shared> $<TARGET_FILE_DIR:SDL_gpu_examples>
        VERBATIM
    )
endif(WIN32)
