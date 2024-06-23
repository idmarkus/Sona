cmake_minimum_required(VERSION 3.25)

set(PLUGIN_NAME "Sona")
set(COMPANY_NAME "Linkable")

# GarageBand 10.3 requires the first letter to be upper-case, and the remaining letters to be lower-case
set(PLUGIN_CODE "Sona")  # A unique four-character plugin id with exactly one upper-case character
set(COMPANY_CODE "LNKB") # A four-character manufacturer id with at least one upper-case character


project(${PLUGIN_NAME} VERSION 0.0.1)

set(JUCE_VST3_COPY_DIR "${CMAKE_BINARY_DIR}/VST3" CACHE STRING "Directory to place .vst3 file after build")

# find_package(JUCE CONFIG REQUIRED)        # If you've installed JUCE to your system
add_subdirectory("JUCE")                    # If you've put JUCE in a subdirectory called JUCE

# If you are building a VST2 or AAX plugin, CMake needs to be told where to find these SDKs on your
# system. This setup should be done before calling `juce_add_plugin`.

# juce_set_vst2_sdk_path(...)
# juce_set_aax_sdk_path(...)

juce_add_plugin(${PLUGIN_NAME}
        # ICON_BIG ...                      # ICON_* arguments specify a path to an image file to use as an icon for the Standalone
        # ICON_SMALL ...
        COMPANY_NAME ${COMPANY_NAME}        # Specify the name of the plugin's author
        IS_SYNTH TRUE                       # Is this a synth or an effect?
        NEEDS_MIDI_INPUT TRUE               # Does the plugin need midi input?
        NEEDS_MIDI_OUTPUT TRUE              # Does the plugin need midi output?
        COPY_PLUGIN_AFTER_BUILD TRUE        # Should the plugin be installed to a default location after building?
        VST3_COPY_DIR ${JUCE_VST3_COPY_DIR}
        PLUGIN_MANUFACTURER_CODE ${COMPANY_CODE}
        PLUGIN_CODE ${PLUGIN_CODE}
        FORMATS VST3                  # The formats to build. Other valid formats are: AAX Unity VST AU AUv3
        PRODUCT_NAME ${PLUGIN_NAME})        # The name of the final executable, which can differ from the target name

# `juce_generate_juce_header` will create a JuceHeader.h for a given target, which will be generated
# into your build tree. This should be included with `#include <JuceHeader.h>`.
juce_generate_juce_header(${PLUGIN_NAME}) # JuceHeader.h is available after first build

## Sources
file(GLOB_RECURSE HEADER_LIST CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/include/*.h*")

target_sources(${PLUGIN_NAME}
        PRIVATE
        src/PluginEditor.cpp
        src/PluginProcessor.cpp
        ${HEADER_LIST})

target_include_directories(${PLUGIN_NAME} PRIVATE "${PROJECT_SOURCE_DIR}/include")

# `target_compile_definitions` adds some preprocessor definitions to our target. In a Projucer
# project, these might be passed in the 'Preprocessor Definitions' field. JUCE modules also make use
# of compile definitions to switch certain features on/off, so if there's a particular feature you
# need that's not on by default, check the module header for the correct flag to set here. These
# definitions will be visible both to your code, and also the JUCE module code, so for new
# definitions, pick unique names that are unlikely to collide! This is a standard CMake command.

target_compile_definitions(${PLUGIN_NAME}
        PUBLIC
        # JUCE_WEB_BROWSER and JUCE_USE_CURL would be on by default, but you might not need them.
        JUCE_WEB_BROWSER=0  # If you remove this, add `NEEDS_WEB_BROWSER TRUE` to the `juce_add_plugin` call
        JUCE_USE_CURL=0     # If you remove this, add `NEEDS_CURL TRUE` to the `juce_add_plugin` call
        JUCE_VST3_CAN_REPLACE_VST2=0)

target_link_libraries(${PLUGIN_NAME}
        PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
        PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)