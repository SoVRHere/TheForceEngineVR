project(TFE_Settings)
set(SOURCES
	gameSourceData.h
	settings.cpp
	settings.h
)

 if(WIN32)
	list(APPEND SOURCES
		windows/registry.cpp
		windows/registry.h
	)
elseif(LINUX)		
list(APPEND SOURCES
		linux/steamlnx.cpp
		linux/steamlnx.h
		linux/vdf_parser.hpp
	)
endif()

#add_custom_target(${PROJECT_NAME} "" SOURCES ${SOURCES})
add_library(${PROJECT_NAME} OBJECT ${SOURCES})
target_sources(${PROJECT_NAME} PRIVATE ${SOURCES})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX Sources FILES ${SOURCES})
