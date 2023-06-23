# CMakeList.txt : CMake project for game, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("game")

# Add source to this project's executable.
add_executable (game "src/main.cpp"  
 "engine/core/ccore.h"
 "engine/core/graphics/ogl_fw/glslprogram.h" 
 "engine/core/graphics/ogl_fw/glslprogram.cpp" 
  
  "engine/core/graphics/camera.h"  )

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET game PROPERTY CXX_STANDARD 20)
endif()

# TODO: Add tests and install targets if needed.

include_directories(engine)

add_subdirectory(engine/submodules/glfw)
add_subdirectory(engine/submodules/glew)
add_subdirectory(engine/submodules/glm)
#add_subdirectory(engine/submodules/cglm)

target_include_directories(game PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/engine/submodules/glfw/include)
target_include_directories(game PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/engine/submodules/glew/include)
target_include_directories(game PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/engine/submodules/glm/include)
#target_include_directories(game PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/engine/submodules/cglm/include)

target_link_libraries(game 
	glfw
	libglew_static
	glm
	#cglm_headers
)