project(TFE_Audio)
set(SOURCES
	MidiSynth/fm4Opl3Device.cpp
	MidiSynth/fm4Opl3Device.h
	MidiSynth/fm4Tables.h
	MidiSynth/opl3.c
	MidiSynth/opl3.h
	MidiSynth/soundFontDevice.cpp
	MidiSynth/soundFontDevice.h
	MidiSynth/Source.txt
	MidiSynth/tsf.h
	audioDevice.cpp
	audioDevice.h
	audioFilters.cpp
	audioFilters.h
	audioOutput.h
	audioSystem.cpp
	audioSystem.h
	midi.h
	midiDevice.h
	midiPlayer.cpp
	midiPlayer.h
	RtMidi.cpp
	RtMidi.h
	systemMidiDevice.cpp
	systemMidiDevice.h
)
if(LINUX AND ENABLE_SYSMIDI)
	# we use the system libraries on Linux
	list(REMOVE_ITEM SOURCES RtMidi.cpp)
	list(REMOVE_ITEM SOURCES RtMidi.h)
endif()
if(NOT ENABLE_SYSMIDI)
	list(REMOVE_ITEM SOURCES systemMidiDevice.cpp)
	list(REMOVE_ITEM SOURCES systemMidiDevice.h)
	if(WIN32)
		list(REMOVE_ITEM SOURCES RtMidi.cpp)
		list(REMOVE_ITEM SOURCES RtMidi.h)
	endif()
endif()

#add_custom_target(${PROJECT_NAME} "" SOURCES ${SOURCES})
add_library(${PROJECT_NAME} OBJECT ${SOURCES})
target_sources(${PROJECT_NAME} PRIVATE ${SOURCES})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX Sources FILES ${SOURCES})
