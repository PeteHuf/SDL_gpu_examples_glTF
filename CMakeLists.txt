# largely based on https://github.com/Ravbug/sdl3-sample

cmake_minimum_required(VERSION 3.16)

# Set C++ version
set(CMAKE_CXX_STANDARD 23)

# set the output directory for built objects.
# This makes sure that the dynamic library goes into the build directory automatically.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

# prevent installing to system directories. 
set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}" CACHE INTERNAL "")

project(SDL_gpu_examples_glTF)

option(ENABLE_DX_DEBUGGING "Enables enhaced debugging with DX12 backend" OFF)

if (APPLE AND NOT CMAKE_SYSTEM_NAME MATCHES "Darwin")
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

if (ENABLE_DX_DEBUGGING)
    add_definitions(-DSDL_GLTF_DX_DEBUGGING)
endif()

# Set the name of the executable
set(EXECUTABLE_NAME SDL_gpu_examples_glTF)

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
        Src/iosLaunchScreen.storyboard
)
# What is iosLaunchScreen.storyboard? This file describes what Apple's mobile platforms
# should show the user while the application is starting up. If you don't include one,
# then you get placed in a compatibility mode that does not allow HighDPI.
# This file is referenced inside Info.plist.in, where it is marked as the launch screen file.
# It is also ignored on non-Apple platforms. 

# To get an app icon on Apple platforms, add it to your executable.
# Afterward, the image file in Info.plist.in.
if(APPLE)
    target_sources("${EXECUTABLE_NAME}" PRIVATE "Src/logo.png")
endif()

# Configure SDL by calling its CMake file.
# we use EXCLUDE_FROM_ALL so that its install targets and configs don't
# pollute upwards into our configuration.
add_subdirectory(External/SDL EXCLUDE_FROM_ALL)

include_directories(External/glm)
include_directories(External/tinygltf)

# Link SDL to our executable. This also makes its include directory available to us. 
target_link_libraries(${EXECUTABLE_NAME}
    SDL3::SDL3
    $<$<BOOL:${ENABLE_DX_DEBUGGING}>:dxgi.lib>
)

target_include_directories(${EXECUTABLE_NAME}
    PRIVATE
    Src
)

# Dealing with assets

# Gather assets
set(ASSETS_LIST "") # Init the assets list
add_subdirectory(Content) # Dive into the `Content` dir

# We have some non-code resources that our application needs in order to work. How we deal with those differs per platform.
if (APPLE)
    # on Apple targets, the application bundle has a "resources" subfolder where we can place our assets.
    # SDL_GetBasePath() gives us easy access to that location.
    set(input_root "${CMAKE_CURRENT_LIST_DIR}")
    macro(add_resource FILE)
        file(RELATIVE_PATH relpath "${input_root}" "${input_root}/${FILE}")
        get_filename_component(relpath "${relpath}" DIRECTORY)
        target_sources(${EXECUTABLE_NAME} PRIVATE ${FILE})
        set_property(SOURCE ${FILE} PROPERTY MACOSX_PACKAGE_LOCATION "Resources/${relpath}")
    endmacro()

    foreach(item IN LISTS ASSETS_LIST)
        add_resource(${item})
    endforeach()
else()
    if (ANDROID)
        if (NOT MOBILE_ASSETS_DIR)
            message(FATAL_ERROR "Building for Android, but MOBILE_ASSETS_DIR is not set")
        endif()
        file(MAKE_DIRECTORY "${MOBILE_ASSETS_DIR}")    # we need to create the project Assets dir otherwise CMake won't copy our assets.
    endif()

    
    macro(copy_helper filename)
        if (ANDROID)
            # MOBILE_ASSETS_DIR is set in the gradle file via the cmake command line and points to the Android Studio Assets folder. 
            # when we copy assets there, the Android build pipeline knows to add them to the apk. 
            set(outname "${MOBILE_ASSETS_DIR}/${filename}")    
        else()
            # for platforms that do not have a good packaging format, all we can do is copy the assets to the process working directory.
            set(outname "${CMAKE_BINARY_DIR}/$<CONFIGURATION>/${filename}")
        endif()
        add_custom_command(POST_BUILD
            TARGET "${EXECUTABLE_NAME}"
		    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_LIST_DIR}/${filename}" "${outname}"
	    )
    endmacro()

    foreach(item IN LISTS ASSETS_LIST)
        copy_helper(${item})
    endforeach()
endif()


# set some extra configs for each platform
set(RESOURCE_FILES "Src/logo.png")
if(IOS)
    set(RESOURCE_FILES "Src/iosLaunchScreen.storyboard;Src/logo.png") # make the splash screen show up on iOS
endif()

set_target_properties(${EXECUTABLE_NAME} PROPERTIES 
    # On macOS, make a proper .app bundle instead of a bare executable
    MACOSX_BUNDLE TRUE
    # Set the Info.plist file for Apple Mobile platforms. Without this file, your app
    # will not launch. 
    MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/Src/Info.plist.in"

    # in Xcode, create a Scheme in the schemes dropdown for the app.
    XCODE_GENERATE_SCHEME TRUE
    # Identification for Xcode
    XCODE_ATTRIBUTE_BUNDLE_IDENTIFIER "com.petehufnagel.sdl-gpu-gltf"
	Xcode_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "com.petehufnagel.sdl-gpu-gltf"
	XCODE_ATTRIBUTE_CURRENTYEAR "${CURRENTYEAR}"
    RESOURCE "${RESOURCE_FILES}"
)

# on Visual Studio, set our app as the default project
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT "${EXECUTABLE_NAME}")

# On macOS Platforms, ensure that the bundle is valid for distribution by calling fixup_bundle.
# note that fixup_bundle does not work on iOS, so you will want to use static libraries 
# or manually copy dylibs and set rpaths
if(CMAKE_SYSTEM_NAME MATCHES "Darwin")
    # tell Install about the target, otherwise fixup won't know about the transitive dependencies
    install(TARGETS ${EXECUTABLE_NAME}
    	BUNDLE DESTINATION install COMPONENT Runtime
   	    RUNTIME DESTINATION install/bin COMPONENT Runtime
    )
	
    set(DEP_DIR "${CMAKE_BINARY_DIR}")  # where to look for dependencies when fixing up
    INSTALL(CODE 
        "include(BundleUtilities)
        fixup_bundle(\"$<TARGET_BUNDLE_DIR:${EXECUTABLE_NAME}>\" \"\" \"${DEP_DIR}\")
        " 
    )
    set(CPACK_GENERATOR "DragNDrop")
    include(CPack)
endif()

if(WIN32)
    # copy the SDL3.dll to the target dir
    add_custom_command(
        TARGET ${EXECUTABLE_NAME} POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy $<TARGET_FILE:SDL3::SDL3-shared> $<TARGET_FILE_DIR:${EXECUTABLE_NAME}>
        VERBATIM
    )
endif(WIN32)
