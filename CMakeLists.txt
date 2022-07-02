cmake_minimum_required (VERSION 3.8)
find_package(Git)

project("StarlightEngineCore")
include(GNUInstallDirs)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_MACOSX_RPATH 1)
cmake_policy(SET CMP0076 NEW)

find_package(Vulkan REQUIRED FATAL_ERROR)

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
	"src/Compiler.cpp" 
	"src/GLSLShader.cpp" 
	"src/ShaderManager.cpp"
	"src/EntityManager.cpp"
	"src/LightManager.cpp"
	"src/MaterialManager.cpp"
	"src/Entity.cpp"
	"src/ObjectManager.cpp"
	"src/Object.cpp"
	"src/TextureManager.cpp"
	"src/BasicVulkanRenderer.cpp"
	#composite files 
	"src/Star_Descriptors.cpp"
	"src/Star_Texture.cpp"
	"src/Star_RenderObject.cpp"
	"src/Star_Buffer.cpp"
	"src/Star_Device.cpp"
	"src/Star_Pipeline.cpp"
	"src/Star_Window.cpp"
	#"src/Star_Swapchain.cpp"
	#rendering sytems
	"src/Systems/StarSystem_RenderObj.cpp"
	"src/Systems/StarSystem_RenderPointLight.cpp"

)
  
SET(${PROJECT_NAME}_HEADERS
	"include/Compiler.h"
	"include/GLSLShader.h"
	"include/ShaderManager.h"
	"include/EntityManager.h"
	"include/LightManager.hpp"
	"include/MaterialManager.hpp"
	"include/Entity.h"
	"include/ObjectManager.hpp"
	"include/Object.h"
	"include/TextureManager.h"
	"include/RenderEngine.h"
	"include/Windowing.h"
	"include/MemoryDescriptor.h"
	"include/BasicVulkanRenderer.h"
	#composite files
	"include/Star_Descriptors.hpp"
	"include/Star_Texture.hpp"
	"include/Star_RenderObject.hpp"
	"include/Star_Buffer.hpp"
	"include/Star_Device.hpp"
	"include/Star_Pipeline.hpp"
	"include/Star_Window.hpp"
	#"include/Star_Swapchain.hpp"
	#rendering systems 
	"include/Systems/StarSystem_RenderObj.hpp"
	"include/Systems/RenderPointLight.hpp"
	#header only files
	"include/VulkanVertex.hpp"
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

add_subdirectory("external/tinyobjloader")

add_library(${PROJECT_NAME} STATIC "${${PROJECT_NAME}_SOURCES}" "include/Image.h" "src/Image.cpp")

#ensure all needed external dependencies are available
if (DEFINED StarlightCommon_SOURCE_DIR)
	set(STARCOMMON_HEADER_DIR ${StarlightCommon_SOURCE_DIR})
endif()
if (IS_DIRECTORY ${STARCOMMON_HEADER_DIR})
	set(StarlightCommon_INCLUDE_DIR ${STARCOMMON_HEADER_DIR}/include)
else()
	message(FATAL_ERROR "StarlightCommon was not found. This is needed for compilation.")
endif()
#glm
if(DEFINED glm_SOURCE_DIR)
    set(GLM_HEADER_DIR ${glm_SOURCE_DIR})
endif()
if (NOT IS_DIRECTORY ${GLM_HEADER_DIR})
    message(FATAL_ERROR "GLM is required. Pull copy into external directory or ensure it is included elsewhere.")
endif()
#glfw
if (NOT DEFINED GLFW_SOURCE_DIR)
	message(FATAL_ERROR "GLFW is required. Ensure copy is available for core.")
endif()
#stb
if (NOT DEFINED STB_DIRECTORY)
	if(IS_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/external/stb)
		set(STB_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/external/stb)
	else()
		message(FATAL_ERROR "Stb is needed for compilation. Pull copy into external directory.")
	endif()
endif()


target_link_libraries(${PROJECT_NAME}
	PRIVATE
		${Vulkan_LIBRARIES}
		shaderc
		tinyobjloader
		glfw
)
message("test: ${StarlightCommon_INCLUDE_DIR}")
target_include_directories(${PROJECT_NAME}
	PRIVATE
		${StarlightCommon_INCLUDE_DIR}
		${STB_DIRECTORY}
	PUBLIC 
		${Vulkan_INCLUDE_DIRS}
		${CMAKE_CURRENT_SOURCE_DIR}/include
		${CMAKE_CURRENT_SOURCE_DIR}/include/Systems
		${CMAKE_CURRENT_SOURCE_DIR}/external/shaderc/libshaderc/include
		#${CMAKE_CURRENT_SOURCE_DIR}/external/tinyobjectloader
		"external/tinyobjloader"
		${GLM_HEADER_DIR}
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