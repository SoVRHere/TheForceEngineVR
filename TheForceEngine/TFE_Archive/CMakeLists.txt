project(TFE_Archive)
set(SOURCES
	zip/miniz.h
	zip/zip.c
	zip/zip.h
	archive.cpp
	archive.h
	CMakeLists.txt
	gobArchive.cpp
	gobArchive.h
	gobMemoryArchive.cpp
	gobMemoryArchive.h
	labArchive.cpp
	labArchive.h
	lfdArchive.cpp
	lfdArchive.h
	zipArchive.cpp
	zipArchive.h
	zstdCompression.cpp
	zstdCompression.h
)

#add_custom_target(${PROJECT_NAME} "" SOURCES ${SOURCES})
add_library(${PROJECT_NAME} OBJECT ${SOURCES})
target_sources(${PROJECT_NAME} PRIVATE ${SOURCES})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX Sources FILES ${SOURCES})
