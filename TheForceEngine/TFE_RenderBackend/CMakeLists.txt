project(TFE_RenderBackend)
set(SOURCES
	Win32OpenGL/dynamicTexture.cpp
	Win32OpenGL/gl.h
	Win32OpenGL/glad.c
	Win32OpenGL/glslParser.cpp
	Win32OpenGL/glslParser.h
	Win32OpenGL/indexBuffer.cpp
	Win32OpenGL/khrplatform.h
	Win32OpenGL/openGL_Caps.cpp
	Win32OpenGL/openGL_Caps.h
	Win32OpenGL/openGL_Debug.cpp
	Win32OpenGL/openGL_Debug.h
	Win32OpenGL/renderBackend.cpp
	Win32OpenGL/renderState.cpp
	Win32OpenGL/renderTarget.cpp
	Win32OpenGL/renderTarget.h
	Win32OpenGL/screenCapture.cpp
	Win32OpenGL/screenCapture.h
	Win32OpenGL/shader.cpp
	Win32OpenGL/shaderBuffer.cpp
	Win32OpenGL/textureGpu.cpp
	Win32OpenGL/vertexBuffer.cpp
	dynamicTexture.h
	indexBuffer.h
	renderBackend.h
	renderState.h
	shader.h
	shaderBuffer.h
	textureGpu.h
	vertexBuffer.h
)

#add_custom_target(${PROJECT_NAME} "" SOURCES ${SOURCES})
add_library(${PROJECT_NAME} OBJECT ${SOURCES})
target_sources(${PROJECT_NAME} PRIVATE ${SOURCES})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX Sources FILES ${SOURCES})
