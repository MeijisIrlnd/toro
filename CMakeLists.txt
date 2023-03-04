cmake_minimum_required(VERSION 3.7.2)
set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "" FORCE)
project(Toro VERSION 0.0.1)
#list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/CMake)
include(FetchContent)
set(FETCH_CONTENT_QUIET OFF)
FetchContent_Declare(
        JUCE
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG origin/master
        GIT_SHALLOW ON
        FIND_PACKAGE_ARGS 7.0.3 GLOBAL
)
#find_package(juce REQUIRED)
FetchContent_MakeAvailable(JUCE)
set(CMAKE_CXX_STANDARD 23)

juce_add_plugin(
        Toro
        PLUGIN_MANUFACTURER_CODE Smpi
        PLUGIN_CODE Smr1
        FORMATS AU VST3 Standalone
        PRODUCT_NAME "Toro"
)
juce_add_module(${CMAKE_CURRENT_SOURCE_DIR}/modules/SDSP)
juce_add_module(${CMAKE_CURRENT_SOURCE_DIR}/modules/SUX)

add_subdirectory(source)
target_sources(Toro PRIVATE
        ${SOURCE}
        )

target_include_directories(Toro PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/include/Asio
        ${CMAKE_CURRENT_SOURCE_DIR}/source)


target_compile_definitions(Toro PUBLIC
        JUCE_ASIO=1
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        AU_MAIN_TYPE=kAudioUnitTypeEffect
        )

#juce_add_binary_data(SprinklerData SOURCES
# Resources/NotoSansJP-Regular.otf
# Resources/SprinklerSliderDark.png
#)
target_link_libraries(Toro PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
        SDSP
        PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)
