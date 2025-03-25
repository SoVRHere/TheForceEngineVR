project(TFE_ForceScript)
set(SOURCES
	Angelscript/add_on/scriptarray/scriptarray.cpp
	Angelscript/add_on/scriptarray/scriptarray.h
	Angelscript/add_on/scriptbuilder/scriptbuilder.cpp
	Angelscript/add_on/scriptbuilder/scriptbuilder.h
	Angelscript/add_on/scriptstdstring/scriptstdstring.cpp
	Angelscript/add_on/scriptstdstring/scriptstdstring.h
	Angelscript/add_on/scriptstdstring/scriptstdstring_utils.cpp
	Angelscript/angelscript/include/angelscript.h
	Angelscript/angelscript/source/as_array.h
	Angelscript/angelscript/source/as_atomic.cpp
	Angelscript/angelscript/source/as_atomic.h
	Angelscript/angelscript/source/as_builder.cpp
	Angelscript/angelscript/source/as_builder.h
	Angelscript/angelscript/source/as_bytecode.cpp
	Angelscript/angelscript/source/as_bytecode.h
	Angelscript/angelscript/source/as_callfunc.cpp
	Angelscript/angelscript/source/as_callfunc.h
	#Angelscript/angelscript/source/as_callfunc_arm.cpp
	#Angelscript/angelscript/source/as_callfunc_arm_gcc.S
	#Angelscript/angelscript/source/as_callfunc_arm_msvc.asm
	#Angelscript/angelscript/source/as_callfunc_arm_vita.S
	#Angelscript/angelscript/source/as_callfunc_arm_xcode.S
	#Angelscript/angelscript/source/as_callfunc_arm64.cpp
	#Angelscript/angelscript/source/as_callfunc_arm64_gcc.S
	#Angelscript/angelscript/source/as_callfunc_arm64_msvc.asm
	#Angelscript/angelscript/source/as_callfunc_arm64_xcode.S
	#Angelscript/angelscript/source/as_callfunc_mips.cpp
	#Angelscript/angelscript/source/as_callfunc_ppc.cpp
	#Angelscript/angelscript/source/as_callfunc_ppc_64.cpp
	#Angelscript/angelscript/source/as_callfunc_sh4.cpp
	#Angelscript/angelscript/source/as_callfunc_x64_gcc.cpp
	#Angelscript/angelscript/source/as_callfunc_x64_mingw.cpp
	#Angelscript/angelscript/source/as_callfunc_x64_msvc.cpp
	#Angelscript/angelscript/source/as_callfunc_x64_msvc_asm.asm
	#Angelscript/angelscript/source/as_callfunc_x86.cpp
	#Angelscript/angelscript/source/as_callfunc_xenon.cpp
	Angelscript/angelscript/source/as_compiler.cpp
	Angelscript/angelscript/source/as_compiler.h
	Angelscript/angelscript/source/as_config.h
	Angelscript/angelscript/source/as_configgroup.cpp
	Angelscript/angelscript/source/as_configgroup.h
	Angelscript/angelscript/source/as_context.cpp
	Angelscript/angelscript/source/as_context.h
	Angelscript/angelscript/source/as_criticalsection.h
	Angelscript/angelscript/source/as_datatype.cpp
	Angelscript/angelscript/source/as_datatype.h
	Angelscript/angelscript/source/as_debug.h
	Angelscript/angelscript/source/as_gc.cpp
	Angelscript/angelscript/source/as_gc.h
	Angelscript/angelscript/source/as_generic.cpp
	Angelscript/angelscript/source/as_generic.h
	Angelscript/angelscript/source/as_globalproperty.cpp
	Angelscript/angelscript/source/as_map.h
	Angelscript/angelscript/source/as_memory.cpp
	Angelscript/angelscript/source/as_memory.h
	Angelscript/angelscript/source/as_module.cpp
	Angelscript/angelscript/source/as_module.h
	Angelscript/angelscript/source/as_namespace.h
	Angelscript/angelscript/source/as_objecttype.cpp
	Angelscript/angelscript/source/as_objecttype.h
	Angelscript/angelscript/source/as_outputbuffer.cpp
	Angelscript/angelscript/source/as_outputbuffer.h
	Angelscript/angelscript/source/as_parser.cpp
	Angelscript/angelscript/source/as_parser.h
	Angelscript/angelscript/source/as_property.h
	Angelscript/angelscript/source/as_restore.cpp
	Angelscript/angelscript/source/as_restore.h
	Angelscript/angelscript/source/as_scriptcode.cpp
	Angelscript/angelscript/source/as_scriptcode.h
	Angelscript/angelscript/source/as_scriptengine.cpp
	Angelscript/angelscript/source/as_scriptengine.h
	Angelscript/angelscript/source/as_scriptfunction.cpp
	Angelscript/angelscript/source/as_scriptfunction.h
	Angelscript/angelscript/source/as_scriptnode.cpp
	Angelscript/angelscript/source/as_scriptnode.h
	Angelscript/angelscript/source/as_scriptobject.cpp
	Angelscript/angelscript/source/as_scriptobject.h
	Angelscript/angelscript/source/as_string.cpp
	Angelscript/angelscript/source/as_string.h
	Angelscript/angelscript/source/as_string_util.cpp
	Angelscript/angelscript/source/as_string_util.h
	Angelscript/angelscript/source/as_symboltable.h
	Angelscript/angelscript/source/as_texts.h
	Angelscript/angelscript/source/as_thread.cpp
	Angelscript/angelscript/source/as_thread.h
	Angelscript/angelscript/source/as_tokendef.h
	Angelscript/angelscript/source/as_tokenizer.cpp
	Angelscript/angelscript/source/as_tokenizer.h
	Angelscript/angelscript/source/as_typeinfo.cpp
	Angelscript/angelscript/source/as_typeinfo.h
	Angelscript/angelscript/source/as_variablescope.cpp
	Angelscript/angelscript/source/as_variablescope.h
	Angelscript/README.txt
	ScriptAPI-Shared/scriptMath.cpp
	ScriptAPI-Shared/scriptMath.h
	ScriptAPI-Shared/scriptTest.cpp
	ScriptAPI-Shared/scriptTest.h
	ScriptAPI-Shared/sharedScriptAPI.cpp
	ScriptAPI-Shared/sharedScriptAPI.h
	float2.cpp
	float2.h
	float2x2.cpp
	float2x2.h
	float3.cpp
	float3.h
	float3x3.cpp
	float3x3.h
	float4.cpp
	float4.h
	float4x4.cpp
	float4x4.h
	forceScript.cpp
	forceScript.h
	scriptInterface.cpp
	scriptAPI.h
	scriptInterface.h
)

set(ARCHFLAGS "${CMAKE_SYSTEM_PROCESSOR}")
#message(STATUS "Arch: "${ARCHFLAGS})

if (APPLE AND NOT IOS)
    # TODO: Would this break building on armv7 Linux?
    if (${CMAKE_SYSTEM_PROCESSOR} MATCHES "arm")
        set(ARCHFLAGS "aarch64")
    endif ()
endif ()

if(${ARCHFLAGS} MATCHES "AMD64")
	enable_language(ASM_MASM)

	add_compile_definitions(ASMJIT_EMBED ASMJIT_STATIC ASMJIT_NO_FOREIGN)

	list(APPEND SOURCES
		Angelscript/angelscript/source/as_callfunc_x64_msvc.cpp
		Angelscript/angelscript/source/as_callfunc_x64_msvc_asm.asm
	)
    set_source_files_properties(
        Angelscript/angelscript/source/as_callfunc_x64_msvc_asm.asm
        PROPERTY LANGUAGE ASM_MASM)
endif()

if(${ARCHFLAGS} MATCHES "^arm")
   if(CMAKE_ASM_COMPILER_WORKS)
       list(APPEND SOURCES
			Angelscript/angelscript/source/as_callfunc_arm.cpp
           Angelscript/angelscript/source/as_callfunc_arm_gcc.S)
       set_property(SOURCE
           Angelscript/angelscript/source/as_callfunc_arm_gcc.S
           APPEND PROPERTY COMPILE_FLAGS " -Wa,-mimplicit-it=always")
   else()
       message(FATAL ERROR "ARM target requires a working assembler")
   endif()
endif()

if(${ARCHFLAGS} MATCHES "^aarch64")
   if(CMAKE_ASM_COMPILER_WORKS)
       if(NOT APPLE)
           list(APPEND SOURCES
				Angelscript/angelscript/source/as_callfunc_arm64.cpp
               Angelscript/angelscript/source/as_callfunc_arm64_gcc.S)
       else()
           list(APPEND SOURCES
               Angelscript/angelscript/source/as_callfunc_arm64_xcode.S)
       endif()
   else()
       message(FATAL ERROR "ARM target requires a working assembler")
   endif()
endif()

#add_custom_target(${PROJECT_NAME} "" SOURCES ${SOURCES})
add_library(${PROJECT_NAME} OBJECT ${SOURCES})
target_sources(${PROJECT_NAME} PRIVATE ${SOURCES})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX Sources FILES ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE "Angelscript/angelscript/include")

