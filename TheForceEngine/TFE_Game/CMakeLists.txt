project(TFE_Game)
set(SOURCES
	igame.cpp
	igame.h
	reticle.cpp
	reticle.h
	saveSystem.cpp
	saveSystem.h
)

#add_custom_target(${PROJECT_NAME} "" SOURCES ${SOURCES})
add_library(${PROJECT_NAME} OBJECT ${SOURCES})
target_sources(${PROJECT_NAME} PRIVATE ${SOURCES})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX Sources FILES ${SOURCES})
