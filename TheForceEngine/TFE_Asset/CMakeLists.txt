project(TFE_Asset)
set(SOURCES
	assetSystem.cpp
	assetSystem.h
	colormapAsset.cpp
	colormapAsset.h
	dfKeywords.cpp
	dfKeywords.h
	fontAsset.cpp
	fontAsset.h
	gameMessages.cpp
	gameMessages.h
	gifWriter.cpp
	gifWriter.h
	gmidAsset.cpp
	gmidAsset.h
	imageAsset.cpp
	imageAsset.h
	levelList.cpp
	levelList.h
	modelAsset_jedi.cpp
	modelAsset_jedi.h
	msf_gif.h
	paletteAsset.cpp
	paletteAsset.h
	spriteAsset.cpp
	spriteAsset.h
	spriteAsset_Jedi.cpp
	spriteAsset_Jedi.h
	textureAsset.cpp
	textureAsset.h
	vocAsset.cpp
	vocAsset.h
	vueAsset.cpp
	vueAsset.h
)

#add_custom_target(${PROJECT_NAME} "" SOURCES ${SOURCES})
add_library(${PROJECT_NAME} OBJECT ${SOURCES})
target_sources(${PROJECT_NAME} PRIVATE ${SOURCES})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX Sources FILES ${SOURCES})

target_link_libraries(${PROJECT_NAME} PRIVATE 
	${CMAKE_SOURCE_DIR}/TheForceEngine/sdl2_win32/lib/x64/SDL2_image.lib
)
