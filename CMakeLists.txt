cmake_minimum_required(VERSION 3.18 FATAL_ERROR)
include(CheckCXXCompilerFlag)

set(CMAKE_DISABLE_SOURCE_CHANGES ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)

# force Release by default.
if(NOT DEFINED CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE "Release" CACHE STRING "" FORCE)
endif()

project(TheForceEngine
	HOMEPAGE_URL "https://theforceengine.github.io"
	DESCRIPTION "Modern 'Jedi Engine' replacement supporting Dark Forces, mods, and in the future Outlaws."
	)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

## gcc-12+ and clang-15+ have a feature to automatically zero all variables/members/...
## this mimics what modern MSVC does.  Enable it for release builds (i.e.
## when not debugging to not hide any real bugs).
if(CMAKE_BUILD_TYPE STREQUAL "Release")
	check_cxx_compiler_flag("-ftrivial-auto-var-init=zero" COMPILER_ENABLE_AUTOZERO)
endif()

# disable Clangs excessive warnings about unhandled switch values
if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-switch")
endif()

if(MSVC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP /EHsc /W3 /GR- /GF")
	# force Unicode over Multi-byte
	#add_definitions(-DUNICODE -D_UNICODE)
	add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
	add_compile_definitions(_HAS_STD_BYTE=0)
	#add_compile_definitions(SDL_MAIN_HANDLED)
endif(MSVC)

if (UNIX AND NOT APPLE)
	set(LINUX ON)
endif()

if(WIN32)
	# windows: drop everything into one folder
	set(CMAKE_INSTALL_BINDIR ".")
	set(CMAKE_INSTALL_LIBDIR ".")
	set(CMAKE_INSTALL_DATADIR ".")
else()
	set(TFE_ICONDIR "share/icons/hicolor")
	# tweak DATADIR to end up with ./share/TheForceEngine/
		set(CMAKE_INSTALL_DATADIR "share/${PROJECT_NAME}"
			CACHE PATH "Read-only architecture-independent data"
		)
endif()
include(GNUInstallDirs)

## Options
option(ENABLE_TFE "Enable building “The Force Engine”" ON)
option(ENABLE_SYSMIDI "Enable System-MIDI Output if RTMidi is available" ON)
option(ENABLE_EDITOR "Enable TFE Editor" OFF)
option(ENABLE_FORCE_SCRIPT "Enable Force Script" OFF)
option(ENABLE_ADJUSTABLEHUD_MOD "Install the build-in “AdjustableHud mod” with TFE" ON)
option(ENABLE_VR "Build TFE with VR Support" ON)
option(START_VR "Start VR on start up, ignored if ENABLE_VR is OFF, use --novr/--vr CLI options to control VR initialization on start up" ON)
if(WIN32)
	option(BUILD_SDL "Build SDL from sources" OFF)
else()
	option(BUILD_SDL "Build SDL from sources" ON)
endif()

if (BUILD_SDL)
# we use function, because it creates a new scope, so we dont have to push and pop modified variables
function(ADD_SDL)
	# see here: https://stackoverflow.com/questions/3766740/overriding-a-default-option-value-in-cmake-from-a-parent-cmakelists-txt
	# This makes sure that we really override the option command in the SDL project
	set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)

	# set(CMAKE_CXX_FLAGS_DEBUG ${CMAKE_CXX_FLAGS_RELEASE}) # always build in release
	#set(SDL_SHARED_ENABLED_BY_DEFAULT OFF)
	#if(WIN32)
	#	set(SDL2_DISABLE_SDL2MAIN OFF)
	#else()
	#endif()
	set(SDL_TEST OFF)
	set(BUILD_SHARED_LIBS OFF)
	#set(SDL_STATIC ON)
	
	add_subdirectory(TheForceEngine/SDL)
	add_subdirectory(TheForceEngine/SDL_image)
endfunction()
add_sdl()
endif()

if(ENABLE_TFE)
	add_executable(tfe)
	
	if(ANDROID)
		add_compile_definitions(SDL_MAIN_HANDLED)
	endif()

	if(START_VR AND ENABLE_VR)
		add_compile_definitions(START_VR)
	endif()
	
	if(ENABLE_VR)
		set_target_properties(tfe PROPERTIES OUTPUT_NAME "TheForceEngineVR")
	else()
		set_target_properties(tfe PROPERTIES OUTPUT_NAME "TheForceEngine")
	endif()

	if(WIN32)
		set_property(TARGET tfe PROPERTY WIN32_EXECUTABLE TRUE)
	endif()

	if(MSVC)
		set_target_properties(tfe PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/TheForceEngine)
	endif()

	if(BUILD_SDL)
		target_link_libraries(tfe PRIVATE SDL2::SDL2-static)
		target_link_libraries(tfe PRIVATE SDL2_image::SDL2_image-static)
		if(WIN32)
			target_link_libraries(tfe PRIVATE SDL2::SDL2main)
		endif()
		include_directories(
			TheForceEngine/SDL/include
			TheForceEngine/SDL_image/include
		)
	elseif(WIN32)
		add_compile_definitions(SDL_SHARED)	
		target_link_libraries(tfe PRIVATE 
			${CMAKE_SOURCE_DIR}/TheForceEngine/sdl2_win32/lib/x64/SDL2.lib
			${CMAKE_SOURCE_DIR}/TheForceEngine/sdl2_win32/lib/x64/SDL2main.lib
			${CMAKE_SOURCE_DIR}/TheForceEngine/sdl2_win32/lib/x64/SDL2_image.lib
		)
		include_directories(
			TheForceEngine/sdl2_win32/include 
		)
	endif()
	
	if(UNIX)
		#find_package(PkgConfig REQUIRED)
		#find_package(SDL2 2.0.20 REQUIRED)
		#pkg_check_modules(SDL2_IMAGE REQUIRED SDL2_image)
		#target_include_directories(tfe PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
		#target_include_directories(tfe PRIVATE ${SDL2_INCLUDE_DIRS})
		#target_include_directories(tfe PRIVATE ${SDL2_IMAGE_INCLUDE_DIRS})
		#target_link_libraries(tfe PRIVATE SDL2::SDL2main SDL2::SDL2
		#			${SDL2_IMAGE_LIBRARIES}
		#)

		# set up build directory to be able to run TFE immediately: symlink
		# the necessary support file directories into the build env.
	endif()

	if(COMPILER_ENABLE_AUTOZERO)
		message(STATUS "enabled -ftrivial-auto-var-init=zero")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ftrivial-auto-var-init=zero")
	endif()

	if(ENABLE_SYSMIDI)
		if(WIN32)
			add_compile_definitions(BUILD_SYSMIDI)
		else()
			# pkg_check_modules(RTMIDI rtmidi>=5.0.0)
			# if(RTMIDI_FOUND)
				add_compile_definitions(BUILD_SYSMIDI)
				# target_link_libraries(tfe PRIVATE ${RTMIDI_LIBRARIES})
			# else()
				# set(ENABLE_SYSMIDI 0)
				# message(STATUS "System MIDI Disabled")
			# endif()
		endif()
	endif()
	
	if(ENABLE_EDITOR)
		add_compile_definitions(BUILD_EDITOR)
	endif()
	if(ENABLE_FORCE_SCRIPT)
		add_compile_definitions(BUILD_FORCE_SCRIPT)
	endif()
	if(ENABLE_VR)
		add_compile_definitions(ENABLE_VR)
	endif()

	if(ENABLE_FORCE_SCRIPT)
		target_include_directories(tfe PRIVATE "TheForceEngine/TFE_ForceScript/Angelscript/angelscript/include")
		target_include_directories(tfe PRIVATE "TheForceEngine/TFE_ForceScript/Angelscript/add_on")
	endif()
	include_directories(TheForceEngine)

	add_subdirectory(TheForceEngine)
endif()

### installation ###

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
	set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/Install" CACHE PATH "..." FORCE)
endif(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)

if(ENABLE_TFE)
	# Main binary
	install(TARGETS tfe
		RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
		BUNDLE  DESTINATION "${CMAKE_INSTALL_BINDIR}"
	)

	# Support data
	set(DATA_DIRS
		"${CMAKE_CURRENT_SOURCE_DIR}/TheForceEngine/Captions"
		"${CMAKE_CURRENT_SOURCE_DIR}/TheForceEngine/Documentation"
		"${CMAKE_CURRENT_SOURCE_DIR}/TheForceEngine/UI_Text"
		"${CMAKE_CURRENT_SOURCE_DIR}/TheForceEngine/UI_Images"
		"${CMAKE_CURRENT_SOURCE_DIR}/TheForceEngine/Shaders"
		"${CMAKE_CURRENT_SOURCE_DIR}/TheForceEngine/SoundFonts"
		"${CMAKE_CURRENT_SOURCE_DIR}/TheForceEngine/Fonts"
	)
	
	if(ENABLE_EDITOR)
		list(APPEND DATA_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/TheForceEngine/EditorDef")
	endif()
	
	install(DIRECTORY
			${DATA_DIRS}
		DESTINATION "${CMAKE_INSTALL_DATADIR}"
		    FILE_PERMISSIONS
		      OWNER_READ OWNER_WRITE
		      GROUP_READ
		      WORLD_READ
		    DIRECTORY_PERMISSIONS
		      OWNER_READ OWNER_EXECUTE OWNER_WRITE
		      GROUP_READ GROUP_EXECUTE GROUP_WRITE
		      WORLD_READ WORLD_EXECUTE
	)
	# Always install the “Mods” directory but not always the “AdjustableHud” mod
	install(DIRECTORY
		DESTINATION "${CMAKE_INSTALL_DATADIR}/Mods"
		DIRECTORY_PERMISSIONS
			OWNER_READ OWNER_EXECUTE OWNER_WRITE
			GROUP_READ GROUP_EXECUTE GROUP_WRITE
			WORLD_READ WORLD_EXECUTE
	)

	# Linux .desktop files
	if(LINUX)
		install(
			FILES "${CMAKE_CURRENT_SOURCE_DIR}/TheForceEngine/io.github.theforceengine.tfe.desktop" DESTINATION "share/applications"
		)
		install(
			FILES "${CMAKE_CURRENT_SOURCE_DIR}/TheForceEngine/io.github.theforceengine.tfe.png" DESTINATION "${TFE_ICONDIR}/256x256/apps"
		)
		install(
			FILES "${CMAKE_CURRENT_SOURCE_DIR}/TheForceEngine/io.github.theforceengine.tfe.metainfo.xml"
			DESTINATION "share/metainfo"
		)
	elseif(WIN32)
		if (NOT BUILD_SDL)
			install(
				FILES 
					"${CMAKE_SOURCE_DIR}/TheForceEngine/SDL2.dll" 
					"${CMAKE_SOURCE_DIR}/TheForceEngine/SDL2_image.dll"
				DESTINATION .
			)
		endif()
		install(
			FILES 
				"${CMAKE_SOURCE_DIR}/TheForceEngine/VrWrapper_d.dll"
			DESTINATION .
			CONFIGURATIONS Debug			
		)
		install(
			FILES
				"${CMAKE_SOURCE_DIR}/TheForceEngine/VrWrapper.dll"
			DESTINATION .
			CONFIGURATIONS Release RelWithDebInfo
		)
	endif()
endif()

if(ENABLE_ADJUSTABLEHUD_MOD)
	add_subdirectory(TheForceEngine/Mods/TFE/AdjustableHud)

	if(LINUX)
		install(
			FILES "${CMAKE_CURRENT_SOURCE_DIR}/TheForceEngine/io.github.theforceengine.tfe.Mod.AdjustableHud.metainfo.xml"
			DESTINATION "share/metainfo"
		)
	endif()
endif()

target_link_libraries(tfe PRIVATE 
	TFE_A11y
	TFE_Archive
	TFE_Asset
	TFE_Audio
	TFE_DarkForces
	TFE_FileSystem
	TFE_FrontEndUI
	TFE_Game
	TFE_Input
	TFE_Jedi
	TFE_Memory
	TFE_Outlaws
	TFE_Polygon
	TFE_PostProcess
	TFE_RenderBackend
	TFE_RenderShared
	TFE_Settings
	TFE_System
	TFE_Ui
)
if(ENABLE_EDITOR)
	target_link_libraries(tfe PRIVATE TFE_Editor)
endif()
if(ENABLE_FORCE_SCRIPT)
	target_link_libraries(tfe PRIVATE TFE_ForceScript)
endif()
if(ENABLE_VR)
	target_link_libraries(tfe PRIVATE TFE_Vr)
endif()
