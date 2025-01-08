project(TFE_FileSystem)
set(SOURCES
	filewriterAsync.cpp
	filewriterAsync.h
	memorystream.cpp
	memorystream.h
)
if(WIN32)
	list(APPEND SOURCES
		filestream.cpp
		filestream.h
		fileutil.cpp
		fileutil.h
		paths.cpp
		paths.h
	)
elseif(LINUX)
	list(APPEND SOURCES
		filestream-posix.cpp
		fileutil-posix.cpp
		paths-posix.cpp
	)
endif()

#add_custom_target(${PROJECT_NAME} "" SOURCES ${SOURCES})
add_library(${PROJECT_NAME} OBJECT ${SOURCES})
target_sources(${PROJECT_NAME} PRIVATE ${SOURCES})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX Sources FILES ${SOURCES})
