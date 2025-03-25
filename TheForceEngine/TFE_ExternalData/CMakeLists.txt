project(TFE_ExternalData)
set(SOURCES
	dfLogics.cpp
	dfLogics.h
	logicTables.cpp
	logicTables.h
	pickupExternal.cpp
	pickupExternal.h
	weaponExternal.cpp
	weaponExternal.h
)

add_library(${PROJECT_NAME} OBJECT ${SOURCES})
target_sources(${PROJECT_NAME} PRIVATE ${SOURCES})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX Sources FILES ${SOURCES})
