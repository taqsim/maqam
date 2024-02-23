# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.22.1)

# Declares and names the project.

project("maqam")

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.

add_library( # Sets the name of the library.
        ${PROJECT_NAME}

        # Sets the library as a shared library.
        SHARED

        # Provides a relative path to your source file(s).
        library.cpp
        client/maqam.c
        impl/AudioGraph.cpp
        impl/AudioNode.cpp
        impl/AudioRoot.cpp
        impl/NativeWrapper.cpp
)

# Searches for a specified prebuilt library and stores the path as a
# variable. Because CMake includes system libraries in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.

find_library( # Sets the name of the path variable.
        log-lib

        # Specifies the name of the NDK library that
        # you want CMake to locate.
        log)

# Include Google Oboe audio library for Android
# https://developer.android.com/games/sdk/oboe/update-build-settings
# https://github.com/google/oboe/blob/main/docs/GettingStarted.md#using-%0Aoboe
find_package (oboe REQUIRED CONFIG)

# Project files location
set(ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../..)

# Include project headers
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Include third-party dependencies
set(THIRDPARTY_DIR ${ROOT_DIR}/thirdparty)
target_include_directories(${PROJECT_NAME} PRIVATE ${THIRDPARTY_DIR})

target_sources(
        ${PROJECT_NAME}
        PRIVATE
        ${THIRDPARTY_DIR}/ring_buffer/ring_buffer.cc
)

# Include JUCE multi-platform audio framework
# https://forum.juce.com/t/native-built-in-cmake-support-in-juce/38700/16
set(JUCE_DIR ${THIRDPARTY_DIR}/JUCE)
add_subdirectory(${JUCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}/JUCE)
add_definitions([[-DJUCE_ANDROID=1]] [[-DJUCE_GLOBAL_MODULE_SETTINGS_INCLUDED=1]])
target_include_directories(${PROJECT_NAME} PRIVATE ${JUCE_DIR}/modules)

# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.

target_link_libraries( # Specifies the target library.
        ${PROJECT_NAME}
        amidi
        oboe::oboe
        juce::juce_audio_processors
        juce::juce_dsp

        # Links the target library to the log library
        # included in the NDK.
        ${log-lib})

# Node implementations
set(NODES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/nodes)
include(${NODES_DIR}/CMakeLists.txt)
