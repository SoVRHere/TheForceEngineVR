project(TFE_Input)
set(SOURCES
	input.cpp
	input.h
	inputEnum.h
	inputMapping.cpp
	inputMapping.h
	replay.cpp
	replay.h
	touchInput.cpp
	touchInput.h
)

#add_custom_target(${PROJECT_NAME} "" SOURCES ${SOURCES})
add_library(${PROJECT_NAME} OBJECT ${SOURCES})
target_sources(${PROJECT_NAME} PRIVATE ${SOURCES})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX Sources FILES ${SOURCES})
