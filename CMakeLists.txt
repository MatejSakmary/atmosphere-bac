cmake_minimum_required(VERSION 3.21)

project(atmosphere VERSION 1.0.0)

macro(compileGlsl GLSL_SOURCES STAGE)
	foreach(GLSL ${GLSL_SOURCES})
  		get_filename_component(FILE_NAME ${GLSL} NAME)
  		set(SPIRV "shaders/build/${FILE_NAME}.spv")
		add_custom_command(
			OUTPUT ${SPIRV}
			COMMAND ${CMAKE_COMMAND} -E make_directory "shaders/build/"
			COMMAND ${GLSLC} -fshader-stage=${STAGE} ${GLSL} -I. -o ${SPIRV}
			WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
			DEPENDS ${GLSL}
		)
  		list(APPEND SPIRV_BINARY_FILES ${SPIRV})
	endforeach(GLSL)
endmacro()

#######################################################################################
# generate shader output
find_program(GLSLC "glslc")
if(NOT GLSLC)
	message(FATAL_ERROR "glslc not found!")
endif()

set (GLSL_VERT_SOURCE_FILES
	"shaders/terrain.vert"
	"shaders/screen_triangle.vert"
)

set (GLSL_FRAG_SOURCE_FILES
	"shaders/draw_far_sky.frag"
	"shaders/draw_clouds.frag"
	"shaders/draw_AE_perspective.frag"
	"shaders/terrain.frag"
	"shaders/final_composition.frag"
)

set(GLSL_COMP_SOURCE_FILES
	"shaders/transmittanceLUT.glsl"
	"shaders/multiscatteringLUT.glsl"
	"shaders/skyviewLUT.glsl"
	"shaders/aerialPerspectiveLUT.glsl"
	"shaders/histogram_generate.glsl"
	"shaders/histogram_sum.glsl"
	"shaders/noise/worley_noise_3D.glsl"
	"shaders/noise/normalize_noise_3D.glsl"
)

compileGlsl("${GLSL_VERT_SOURCE_FILES}" "vert")
compileGlsl("${GLSL_FRAG_SOURCE_FILES}" "frag")
compileGlsl("${GLSL_COMP_SOURCE_FILES}" "comp")

add_custom_target(
    Shaders 
    DEPENDS ${SPIRV_BINARY_FILES}
)
#######################################################################################

add_library(stb STATIC
    "source/dep/stb_image/stb_image.cpp"
    "source/dep/stb_image/stb_image_write.cpp"
)

add_library(miniz STATIC "source/dep/miniz/miniz.c")

add_library(tinyexr STATIC "source/dep/tinyexr/tinyexr.cpp")
target_include_directories(tinyexr PRIVATE "source/dep/miniz")
target_link_libraries(tinyexr PRIVATE miniz)

# Set GLFW variables so that we don't build GLFW test etc
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory("source/dep/glfw")

add_library(imgui STATIC
    "source/dep/imgui/imgui_demo.cpp"
    "source/dep/imgui/imgui_draw.cpp"
    "source/dep/imgui/imgui_tables.cpp"
    "source/dep/imgui/imgui_widgets.cpp"
    "source/dep/imgui/imgui.cpp"
    "source/dep/imgui/backends/imgui_impl_glfw.cpp"
    "source/dep/imgui/backends/imgui_impl_vulkan.cpp"
)

target_include_directories(imgui PRIVATE
    "source/dep/imgui"
)

target_link_libraries(imgui PUBLIC Vulkan::Vulkan glfw)


add_executable(${PROJECT_NAME}
    "source/main.cpp"
    "source/application.cpp"
    "source/camera.cpp"
    "source/vulkan/renderer.cpp"
    "source/vulkan/imgui_impl.cpp"
    "source/vulkan/vulkan_buffer.cpp"
    "source/vulkan/vulkan_debug.cpp"
    "source/vulkan/vulkan_device.cpp"
    "source/vulkan/vulkan_image.cpp"
    "source/vulkan/vulkan_pipeline.cpp"
    "source/vulkan/vulkan_swapchain.cpp"
    "source/noise/worley_noise.cpp"
    "source/model/sky_model.cpp"
)

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)

target_include_directories(${PROJECT_NAME}
    PRIVATE
    "source"
    "source/dep/stb_image"
    "source/dep/tinyexr"
    "source/dep/imgui"
)

find_package(Vulkan REQUIRED)

add_subdirectory("source/dep/glm")

target_link_libraries(${PROJECT_NAME} PRIVATE Vulkan::Vulkan glfw glm stb tinyexr imgui)

add_dependencies(${PROJECT_NAME} Shaders)