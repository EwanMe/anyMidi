cmake_minimum_required(VERSION 3.15)

project(anyMidi VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(JUCE_REQUIRED_VERSION 7.0.9)
if(CMAKE_PREFIX_PATH)
	find_package(JUCE ${JUCE_REQUIRED_VERSION} CONFIG REQUIRED)
elseif(JUCE_PATH)
	add_subdirectory(${JUCE_PATH} "${CMAKE_CURRENT_BINARY_DIR}/JUCEBuild")
	
	# Get JUCE version
	file(READ "${JUCE_PATH}/CMakeLists.txt" CONTENTS)
	if (${CONTENTS} MATCHES "project\\(JUCE[^)]+VERSION +([0-9]+\\.[0-9]+\\.[0-9]+)")
		set(JUCE_VERSION ${CMAKE_MATCH_1} CACHE INTERNAL "Project version")
		if(JUCE_VERSION STREQUAL JUCE_REQUIRED_VERSION)
			message(STATUS "Found JUCE version: ${JUCE_VERSION}")
		else()
			message(FATAL_ERROR "Incorrect JUCE version. "
			"Found ${JUCE_VERSION}, expected ${JUCE_REQUIRED_VERSION}")
		endif()
	else()
		message(WARNING
		"Version information not found in ${JUCE_PATH}/CMakeLists.txt, "
		"assuming correct version ${JUCE_REQUIRED_VERSION} installed.")
	endif()
else()
	message(FATAL_ERROR
	"No JUCE location provided. Please specify CMAKE_PREFIX_PATH for library "
	"location or JUCE_PATH for source code location.")
endif()

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

if (UNIX AND RUN_CLANG_TIDY)
	# Run clang-tidy only on linux
	foreach(version RANGE 15 17)
		list(APPEND CLANG_TIDY_VERSIONS "clang-tidy-${version}")	
	endforeach()

	find_program(CLANG_TIDY_BIN NAMES ${CLANG_TIDY_VERSIONS})

	if(CLANG_TIDY_BIN)
		message(STATUS "Found ${CLANG_TIDY_BIN}")
	else()
		message(FATAL_ERROR "Could not find clang-tidy with version >14")
	endif()

	set(HEADERS_TO_TIDY
		".*src/core/AudioProcessor\.h"
		".*src/core/ForwardFFT\.h"
		".*src/core/MidiProcessor\.h"
		".*src/ui/CustomLookAndFeel\.h"
		".*src/ui/MainComponent\.h"
		".*src/ui/UserInterface\.h"
	)

	string(JOIN "|" HEADER_FILTER ${HEADERS_TO_TIDY})

	set_target_properties(anyMidi PROPERTIES
		CXX_CLANG_TIDY "${CLANG_TIDY_BIN};-header-filter=^(${HEADER_FILTER})$"
	)
endif()

set(PRIVATE_LIBS
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
)

set(PUBLIC_LIBS
	juce::juce_recommended_config_flags
	juce::juce_recommended_lto_flags
	juce::juce_recommended_warning_flags
)

if(UNIX)
	set(PRIVATE_LIBS curl ${PRIVATE_LIBS})
	target_compile_options(anyMidi
		PRIVATE
			-I/usr/include/atk-1.0
			-I/usr/include/cairo
			-I/usr/include/harfbuzz
			-I/usr/include/gdk-pixbuf-2.0
			-I/usr/include/glib-2.0
			-I/usr/include/gtk-3.0
			-I/usr/include/libsoup-2.4
			-I/usr/include/pango-1.0
			-I/usr/include/webkitgtk-4.0
			-I/usr/lib/x86_64-linux-gnu/glib-2.0/include
	)
endif()

target_include_directories(anyMidi SYSTEM PRIVATE
	"${CMAKE_PREFIX_PATH}/include/JUCE-${JUCE_VERSION}/modules/")

target_link_libraries(anyMidi
	PRIVATE
		${PRIVATE_LIBS}
	PUBLIC
		${PUBLIC_LIBS}
)
