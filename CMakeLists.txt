cmake_minimum_required(VERSION 3.15)

project(anyMidi VERSION 1.0.0)


set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(JUCE CONFIG REQUIRED)

juce_add_gui_app(anyMidi
	VERSION 1.0.0
	PRODUCT_NAME anyMidi)

juce_add_binary_data(anyMidiAssets
	SOURCES
		"${CMAKE_SOURCE_DIR}/resources/anyMidiLogo.png"
		"${CMAKE_SOURCE_DIR}/resources/anyMidiLogoSmall.png"
)

file(GLOB_RECURSE SRC_FILES
	"src/core/AudioProcessor.cpp"
	"src/core/ForwardFFT.cpp"
	"src/core/MidiProcessor.cpp"
	"src/ui/CustomLookAndFeel.cpp"
	"src/ui/MainComponent.cpp"
	"src/ui/UserInterface.cpp"
	"src/Main.cpp"	
)

target_sources(anyMidi PRIVATE ${SRC_FILES})

#set_target_properties(tidy PROPERTIES
#    CXX_CLANG_TIDY "clang-tidy;--header-filter=^(!?${CMAKE_SOURCE_DIR}\\Builds|${CMAKE_PREFIX_PATH})"
#)

target_link_libraries(anyMidi
	PRIVATE
		anyMidiAssets
		juce::juce_audio_basics
		juce::juce_audio_devices
		juce::juce_audio_formats
		juce::juce_audio_processors
		juce::juce_audio_utils
		juce::juce_core
		juce::juce_data_structures
		juce::juce_dsp
		juce::juce_events
		juce::juce_graphics
		juce::juce_gui_basics
		juce::juce_gui_extra
	PUBLIC
		juce::juce_recommended_config_flags
		juce::juce_recommended_lto_flags
		juce::juce_recommended_warning_flags
)

juce_generate_juce_header(anyMidi)
