project(TFE_FrontEndUI)
set(SOURCES
	console.cpp
	console.h
	frontEndUi.cpp
	frontEndUi.h
	modLoader.cpp
	modLoader.h
	profilerView.cpp
	profilerView.h
	uiTexture.cpp
	uiTexture.h
)

#add_custom_target(${PROJECT_NAME} "" SOURCES ${SOURCES})
add_library(${PROJECT_NAME} OBJECT ${SOURCES})
target_sources(${PROJECT_NAME} PRIVATE ${SOURCES})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX Sources FILES ${SOURCES})
