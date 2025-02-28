project(TFE_Vr)

option(VRW_BUILD_IN_VR_WRAPPER OFF)

set(VR_WRAPPER_PATH "VrWrapper")

set(SOURCES
	vr.cpp
	vr.h
	${VR_WRAPPER_PATH}/VrWrapper.h
)

if(VRW_BUILD_IN_VR_WRAPPER)
list(APPEND SOURCES
	${VR_WRAPPER_PATH}/VrWrapper.cpp
	${VR_WRAPPER_PATH}/VrMath.cpp
	${VR_WRAPPER_PATH}/VrMath.h
	${VR_WRAPPER_PATH}/Message.h
	${VR_WRAPPER_PATH}/Instance.h
	
	${VR_WRAPPER_PATH}/GL/OpenGLDebug.cpp
	${VR_WRAPPER_PATH}/GL/gl.h
	${VR_WRAPPER_PATH}/GL/khrplatform.h
	${VR_WRAPPER_PATH}/GL/OpenGLDebug.h
	
	${VR_WRAPPER_PATH}/OpenXR/OpenXRWrapper.cpp
	${VR_WRAPPER_PATH}/OpenXR/OpenXRWrapper.h
	${VR_WRAPPER_PATH}/OpenXR/OpenXRDebug.cpp
	${VR_WRAPPER_PATH}/OpenXR/OpenXRDebug.h
)
endif()

#add_custom_target(${PROJECT_NAME} "" SOURCES ${SOURCES})
add_library(${PROJECT_NAME} OBJECT ${SOURCES})
target_sources(${PROJECT_NAME} PRIVATE ${SOURCES})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX Sources FILES ${SOURCES})

if(VRW_BUILD_IN_VR_WRAPPER)
	add_compile_definitions(VRW_BUILD_IN_VR_WRAPPER)
	target_include_directories(${PROJECT_NAME} PRIVATE
		${VR_WRAPPER_PATH}/
		${VR_WRAPPER_PATH}/External
		${VR_WRAPPER_PATH}/OpenXR
	)
	target_include_directories(${PROJECT_NAME} PRIVATE "../SDL/include")
	if(WIN32)
		target_link_directories(${PROJECT_NAME} PRIVATE ${VR_WRAPPER_PATH}/Lib/Windows/x64)
		target_link_libraries(${PROJECT_NAME} PRIVATE optimized openxr_loader.lib debug openxr_loaderd.lib)
		target_link_libraries(${PROJECT_NAME} PRIVATE opengl32.lib)
	elseif(ANDROID)
		# target_link_directories(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/${VR_WRAPPER_PATH}/Lib/Android/${ANDROID_ABI}")
		# target_link_libraries(${PROJECT_NAME} PRIVATE openxr_loader)
		# target_link_libraries(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/${VR_WRAPPER_PATH}/Lib/Android/arm64-v8a/openxr_loader)
	endif()
else()
	if(WIN32)
		#target_link_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/VrWrapper/Lib/Windows/x64)
		target_link_libraries(${PROJECT_NAME} PRIVATE 
			optimized ${CMAKE_CURRENT_SOURCE_DIR}/${VR_WRAPPER_PATH}/Lib/Windows/x64/VrWrapper.lib 
			debug ${CMAKE_CURRENT_SOURCE_DIR}/${VR_WRAPPER_PATH}/Lib/Windows/x64/VrWrapper_d.lib)
	elseif(ANDROID)
		target_link_libraries(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/${VR_WRAPPER_PATH}/Lib/Android/arm64-v8a/libopenxr_loader.so)
		target_link_libraries(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/${VR_WRAPPER_PATH}/Lib/Android/arm64-v8a/libVrWrapper.so)
	endif()
endif()
