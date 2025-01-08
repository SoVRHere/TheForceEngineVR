project(TFE_System)
set(SOURCES
	CrashHandler/crashHandler.h 
	cJSON.c
	frameLimiter.cpp
	iniParser.cpp
	log.cpp
	math.cpp
	memoryPool.cpp
	parser.cpp
	profiler.cpp
	system.cpp
	tfeMessage.cpp
	utf8.cpp
	cJSON.h
	frameLimiter.h
	iniParser.h
	math.h
	memoryPool.h
	parser.h
	profiler.h
	system.h
	tfeMessage.h
	types.h
	utf8.h
)
if(WIN32)
	list(APPEND SOURCES CrashHandler/crashHandlerWin32.cpp)
elseif(LINUX)
	list(APPEND SOURCES CrashHandler/crashHandlerLinux.cpp)
endif()

#add_custom_target(${PROJECT_NAME} "" SOURCES ${SOURCES})
add_library(${PROJECT_NAME} OBJECT ${SOURCES})
target_sources(${PROJECT_NAME} PRIVATE ${SOURCES})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX Sources FILES ${SOURCES})
