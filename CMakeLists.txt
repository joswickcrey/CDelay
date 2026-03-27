cmake_minimum_required(VERSION 3.22)
project(CDelay VERSION 0.0.1)

set(JUCE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/JUCE" CACHE PATH "Path to JUCE")
add_subdirectory(${JUCE_PATH} JUCE)

juce_add_plugin(CDelay
    COMPANY_NAME "YourCompany"
    PLUGIN_MANUFACTURER_CODE Ycmp
    PLUGIN_CODE CDly
    FORMATS VST3 AU
    PRODUCT_NAME "CDelay"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    VST3_CAN_REPLACE_VST2 FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    VST3_CATEGORIES "Fx|Delay"
)

juce_generate_juce_header(CDelay)

target_sources(CDelay
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp
)

target_compile_features(CDelay PUBLIC cxx_std_17)

target_link_libraries(CDelay
    PRIVATE
        juce::juce_audio_utils
        juce::juce_audio_processors
        juce::juce_audio_basics
        juce::juce_core
        juce::juce_data_structures
        juce::juce_events
        juce::juce_graphics
        juce::juce_gui_basics
	juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

target_compile_definitions(CDelay
    PUBLIC
        JUCE_VST3_CAN_REPLACE_VST2=0
)