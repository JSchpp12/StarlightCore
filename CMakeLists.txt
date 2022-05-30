cmake_minimum_required (VERSION 3.8)
find_package(Git)

project("StarlightEngineCore")
include(GNUInstallDirs)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_MACOSX_RPATH 1)
cmake_policy(SET CMP0076 NEW)

if (GIT_FOUND)
	if(GIT_SUBMODULE AND PULL_SUBMODULE)
		message(STATUS "Submodule update")
		execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
			WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
			RESULT_VARIABLE GIT_SUBMOD_RESULT)
		if(NOT GIT_SUBMOD_RESULT EQUAL "0")
			message(FATAL_ERROR "git submodule update --init --recursive failed with ${GIT_SUBMOD_RESULT}, please checkout submodules")
		endif()
	endif()

	if(PREP_EXTERNAL_PACKAGES)
		message("Preparing requirenments for external dependencies")
		message("Preparing dependencies for spirvtools")
		find_library(EFFCEE_LIB NAME effcee HINTS ${CMAKE_CURRENT_SOURCE_DIR}/external/spirvtools/external)
		if (NOT EFFCEE_LIB AND NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/external/spirvtools/external/effcee")
			message("effcee is required for spirvtools and was not found, fetching...")
			execute_process(COMMAND ${GIT_EXECUTABLE} clone https://github.com/google/effcee.git
				WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/external/spirvtools/external"
				RESULT_VARIABLE GIT_EFFCEE_RESULT
			)
			if (NOT GIT_EFFCEE_RESULT EQUAL "0")
				message(FATAL_ERROR "effcee is needed for building spirvtools, fetch into required dir")
			endif()
		endif()

		find_library(RE2_LIB NAME re2 HINTS ${CMAKE_CURRENT_SOURCE_DIR}/external/spirvtools/external)
		if (NOT RE2_LIB AND NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/external/spirvtools/external/re2")
			message("re2 is required for spirvtools and was not found, fetching...")
			execute_process(COMMAND ${GIT_EXECUTABLE} clone https://github.com/google/re2.git
				WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/external/spirvtools/external"
				RESULT_VARIABLE GIT_RE2_RESULT
			)
			if (NOT GIT_RE2_RESULT EQUAL "0")
				message(FATAL_ERROR "re2 is needed for building spirvtools, fetch into required dir")
			endif()
		endif()
	endif()
	message("Prep Complete")
endif()

SET(${PROJECT_NAME}_SOURCES
	"src/ShaderManager/Compiler.cpp" 
	"src/ShaderManager/GLSLShader.cpp" 
	"src/ShaderManager/ShaderManager.cpp"
)

SET(${PROJECT_NAME}_HEADERS
	"include/ShaderManager/Compiler.h"
	"include/ShaderManager/GLSLShader.h"
	"include/ShaderManager/ShaderManager.h"
)

SET(${${PROJECT_NAME}_HEADER_ONLY}
	""
)

SET(${PROJECT_NAME}_INCLUDE "${PROJECT_SOURCE_DIR}/src")

set(SPIRV_HEADERS_SKIP_EXAMPLES ON CACHE BOOL "" FORCE)
add_subdirectory("external/spirvheaders")

set(SPIRV_SKIP_EXECUTABLES ON CACHE BOOL "" FORCE)
set(SPIRV_SKIP_TESTS ON CACHE BOOL "" FORCE)
add_subdirectory("external/spirvtools")

set(ENABLE_CTEST OFF CACHE BOOL "" FORCE)
set(ENABLE_GLSLANG_BINARIES OFF CACHE BOOL "" FORCE)
add_subdirectory("external/glslang")

set(SHADERC_SKIP_EXAMPLES ON CACHE BOOL "" FORCE)
set(SHADERC_SKIP_TESTS ON CACHE BOOL "" FORCE)
set(SHADERC_ENABLE_SHARED_CRT ON CACHE BOOL "" FORCE)
set(SHADERC_SKIP_COPYRIGHT_CHECK ON CACHE BOOL "" FORCE)
add_subdirectory("external/shaderc")

add_library(${PROJECT_NAME} STATIC "${${PROJECT_NAME}_SOURCES}")

#TODO: move to external file 
#check for starlight common 
if (DEFINED StarlightCommon_SOURCE_DIR)
	set(STARCOMMON_HEADER_DIR ${StarlightCommon_SOURCE_DIR})
endif()

if (IS_DIRECTORY ${STARCOMMON_HEADER_DIR})
	set(StarlightCommon_INCLUDE_DIR ${STARCOMMON_HEADER_DIR}/include)
else()
	message(FATAL_ERROR "StarlightCommon was not found. This is needed for compilation.")
endif()

target_link_libraries(${PROJECT_NAME}
	PRIVATE
		shaderc
)
message("test: ${StarlightCommon_INCLUDE_DIR}")
target_include_directories(${PROJECT_NAME}
	PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}/include/Shadermanager
		${StarlightCommon_INCLUDE_DIR}
	PUBLIC 
		${CMAKE_CURRENT_SOURCE_DIR}/include
		${CMAKE_CURRENT_SOURCE_DIR}/external/shaderc/libshaderc/include
		#$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
		#$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/interface>
		#$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
#[[
target_include_directories(${PROJECT_NAME}
	PUBLIC 
		${CMAKE_CURRENT_SOURCE_DIR}/src
		#${PROJECT_SOURCE_DIR}/include
		#$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include> 
		#${PROJECT_SOURCE_DIR}/external/shaderc/libshaderc/include
	PRIVATE 
		${CMAKE_CURRENT_SOURCE_DIR}
		${CMAKE_CURRENT_SOURCE_DIR}/external/shaderc/libshaderc/include
)

	PUBLIC
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
		$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
]]


#[[
target_sources(${PROJECT_NAME}
    PUBLIC 
		${${PROJECT_NAME}_HEADER_ONLY}
        ${${PROJECT_NAME}_SOURCES}
        ${${PROJECT_NAME}_HEADERS}
)]]

# install(
# 	FILES 
# 		${${PROJECT_NAME}_HEADERS}
# 	DESTINATION 
# 		${CMAKE_INSTALL_LIBDIR}/starenginecore
# )

set_target_properties(${PROJECT_NAME} PROPERTIES DEBUG_POSTFIX "d")