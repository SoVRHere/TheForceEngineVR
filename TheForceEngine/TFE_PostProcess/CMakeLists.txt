project(TFE_PostProcess)
set(SOURCES
	blit.cpp
	blit.h
	bloomDownsample.cpp
	bloomDownsample.h
	bloomMerge.cpp
	bloomMerge.h
	bloomThreshold.cpp
	bloomThreshold.h
	overlay.cpp
	overlay.h
	postprocess.cpp
	postprocess.h
	postprocesseffect.h
)

#add_custom_target(${PROJECT_NAME} "" SOURCES ${SOURCES})
add_library(${PROJECT_NAME} OBJECT ${SOURCES})
target_sources(${PROJECT_NAME} PRIVATE ${SOURCES})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX Sources FILES ${SOURCES})
