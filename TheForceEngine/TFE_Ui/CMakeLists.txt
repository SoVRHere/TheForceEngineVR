project(TFE_Ui)
set(SOURCES
	imGUI/Dirent/dirent.h
	imGUI/imconfig.h
	imGUI/imgui.cpp
	imGUI/imgui.h
	#imGUI/imgui_demo.cpp
	imGUI/imgui_draw.cpp
	#imGUI/imgui_impl_opengl3.cpp
	#imGUI/imgui_impl_opengl3.h
	#imGUI/imgui_impl_opengl3_loader.h
	imGUI/imgui_impl_sdl2.cpp
	imGUI/imgui_impl_sdl2.h
	imGUI/imgui_internal.h
	imGUI/imgui_markdown.h
	imGUI/imgui_tables.cpp
	imGUI/imgui_tfe_widgets.cpp
	imGUI/imgui_tfe_widgets.h
	imGUI/imgui_widgets.cpp
	imGUI/imstb_rectpack.h
	imGUI/imstb_textedit.h
	imGUI/imstb_truetype.h
	markdown.cpp
	markdown.h
	portable-file-dialogs.h
	ui.cpp
	ui.h
)

#add_custom_target(${PROJECT_NAME} "" SOURCES ${SOURCES})
add_library(${PROJECT_NAME} OBJECT ${SOURCES})
target_sources(${PROJECT_NAME} PRIVATE ${SOURCES})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX Sources FILES ${SOURCES})

