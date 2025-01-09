project(TFE_Vr)
set(SOURCES
	vr.cpp
	vr.h
	VrWrapper.h
)

#add_custom_target(${PROJECT_NAME} "" SOURCES ${SOURCES})
add_library(${PROJECT_NAME} OBJECT ${SOURCES})
target_sources(${PROJECT_NAME} PRIVATE ${SOURCES})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX Sources FILES ${SOURCES})

target_link_libraries(${PROJECT_NAME} PRIVATE 
	optimized ${CMAKE_SOURCE_DIR}/TheForceEngine/TFE_Vr/Lib/x64/VrWrapper.lib
	debug ${CMAKE_SOURCE_DIR}/TheForceEngine/TFE_Vr/Lib/x64/VrWrapper_d.lib
)
