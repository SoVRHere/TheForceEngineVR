project(TFE_RenderShared)
set(SOURCES
	camera3d.h
	CMakeLists.txt
	gridDef.h
	imGuiDraw.cpp
	imGuiDraw.h
	lineDraw2d.cpp
	lineDraw2d.h
	lineDraw3d.cpp
	lineDraw3d.h
	lineDrawMode.cpp
	lineDrawMode.h
	modelDraw.cpp
	modelDraw.h
	quadDraw2d.cpp
	quadDraw2d.h
	texturePacker.cpp
	texturePacker.h
	triDraw2d.cpp
	triDraw2d.h
	triDraw3d.cpp
	triDraw3d.h
)

#add_custom_target(${PROJECT_NAME} "" SOURCES ${SOURCES})
add_library(${PROJECT_NAME} OBJECT ${SOURCES})
target_sources(${PROJECT_NAME} PRIVATE ${SOURCES})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX Sources FILES ${SOURCES})
