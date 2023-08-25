cmake_minimum_required(VERSION 3.24)
project(Realtime_Cell_Collapse)

set(CMAKE_CXX_STANDARD 20)

add_link_options("-v")
set(CMAKE_VERBOSE_MAKEFILE ON)

# flags
#add_definitions("-DGLM_FORCE_MESSAGES")
#add_definitions("-DGLM_FORCE_DEPTH_ZERO_TO_ONE")
#set(GLM_FORCE_MESSAGES ON)
#vcpkg_configure_cmake(
#        SOURCE_PATH ${SOURCE_PATH}
#        OPTIONS
#            -DGLM_FORCE_MESSAGES=ON
#)

if (WIN32)
    message(STATUS "Windows mode")
    set(VK_FOLDER "C:/VulkanSDK/1.3.239.0")
elseif (LINUX)
    message(STATUS "Linux mode")
    set(VK_FOLDER "$ENV{HOME}/VulkanSDK/1.3.239.0")
elseif (UNIX)
    message(STATUS "Unix mode")
    set(VK_FOLDER "$ENV{HOME}/VulkanSDK/1.3.239.0/macOS")
    message(STATUS "VULKAN_SDK: $ENV{VULKAN_SDK}")
endif ()

set(HEADER_FOLDER include)
set(SOURCE_FOLDER source)
set(EXTERNAL_FOLDER external)
set(LIB_FOLDER lib)

# Add files
# Add any added source/header files here
set(HEADER_FILES
        ${HEADER_FOLDER}/application.h

        ${HEADER_FOLDER}/graphics/renderer.h
        ${HEADER_FOLDER}/graphics/colors.h
        ${HEADER_FOLDER}/graphics/vertex.h
        ${HEADER_FOLDER}/graphics/uniform_buffer_object.h
        ${HEADER_FOLDER}/graphics/render_mesh.h
        ${HEADER_FOLDER}/graphics/pnext_chain_reader.h
        ${HEADER_FOLDER}/graphics/projector.h
        ${HEADER_FOLDER}/graphics/triangle.h

        ${HEADER_FOLDER}/graphics/vulkan/vulkan_images.h
        ${HEADER_FOLDER}/graphics/vulkan/vulkan_memory.h
        ${HEADER_FOLDER}/graphics/vulkan/vulkan_buffers.h
        ${HEADER_FOLDER}/graphics/vulkan/vulkan_devices.h
        ${HEADER_FOLDER}/graphics/vulkan/vulkan_instance.h
        ${HEADER_FOLDER}/graphics/vulkan/vulkan_validation.h
        ${HEADER_FOLDER}/graphics/vulkan/vulkan_swapchain.h
        ${HEADER_FOLDER}/graphics/vulkan/vulkan_renderpasses.h

        ${HEADER_FOLDER}/io/input_manager.h
        ${HEADER_FOLDER}/io/window_manager.h
        ${HEADER_FOLDER}/io/printer.h

        ${HEADER_FOLDER}/ecs/ecs.h
        ${HEADER_FOLDER}/ecs/entity.h
        ${HEADER_FOLDER}/ecs/entities/dense_sphere.h
        ${HEADER_FOLDER}/ecs/entities/world.h
        ${HEADER_FOLDER}/ecs/entities/camera.h
        ${HEADER_FOLDER}/ecs/systems/camera_controller.h
        ${HEADER_FOLDER}/ecs/systems/sphere_controller.h

        ${HEADER_FOLDER}/physics/transformer.h

        ${HEADER_FOLDER}/util/timer.h
        ${HEADER_FOLDER}/util/importer.h
        ${HEADER_FOLDER}/util/byte_size.h
        ${HEADER_FOLDER}/util/glm_include.h
        ${HEADER_FOLDER}/util/glfw_include.h)
message(STATUS "HEADER_FILES: ${HEADER_FILES}")
set(SOURCE_FILES
        ${SOURCE_FOLDER}/application.cpp

        ${SOURCE_FOLDER}/graphics/renderer/drawing.cpp
        ${SOURCE_FOLDER}/graphics/renderer/common.cpp
        ${SOURCE_FOLDER}/graphics/renderer/shaders.cpp
        ${SOURCE_FOLDER}/graphics/renderer/systems.cpp
        ${SOURCE_FOLDER}/graphics/colors.cpp
        ${SOURCE_FOLDER}/graphics/projector.cpp

        ${SOURCE_FOLDER}/graphics/vulkan/vulkan_buffers.cpp
        ${SOURCE_FOLDER}/graphics/vulkan/vulkan_images.cpp
        ${SOURCE_FOLDER}/graphics/vulkan/vulkan_memory.cpp
        ${SOURCE_FOLDER}/graphics/vulkan/vulkan_devices.cpp
        ${SOURCE_FOLDER}/graphics/vulkan/vulkan_instance.cpp
        ${SOURCE_FOLDER}/graphics/vulkan/vulkan_validation.cpp
        ${SOURCE_FOLDER}/graphics/vulkan/vulkan_swapchain.cpp
        ${SOURCE_FOLDER}/graphics/vulkan/vulkan_renderpasses.cpp

        ${SOURCE_FOLDER}/io/input_manager.cpp
        ${SOURCE_FOLDER}/io/window_manager.cpp

        ${SOURCE_FOLDER}/ecs/ecs.cpp
        ${SOURCE_FOLDER}/ecs/entity.cpp
        ${SOURCE_FOLDER}/ecs/entities/dense_sphere.cpp
        ${SOURCE_FOLDER}/ecs/entities/world.cpp
        ${SOURCE_FOLDER}/ecs/entities/camera.cpp
        ${SOURCE_FOLDER}/ecs/systems/camera_controller.cpp
        ${SOURCE_FOLDER}/ecs/systems/sphere_controller.cpp

        ${SOURCE_FOLDER}/physics/transformer.cpp

        ${SOURCE_FOLDER}/util/importer.cpp)
message(STATUS "SOURCE_FILES: ${SOURCE_FILES}")
# Main executable
add_executable(Realtime_Cell_Collapse ${SOURCE_FOLDER}/main.cpp ${HEADER_FILES} ${SOURCE_FILES})

# Add folders
target_include_directories(Realtime_Cell_Collapse PUBLIC ${HEADER_FOLDER})
target_include_directories(Realtime_Cell_Collapse PUBLIC ${SOURCE_FOLDER})

# vcpkg autogen
# glfw
find_package(glfw3 CONFIG REQUIRED)
target_link_libraries(Realtime_Cell_Collapse PRIVATE glfw)
# assimp
find_package(assimp CONFIG REQUIRED)
target_link_libraries(Realtime_Cell_Collapse PRIVATE assimp::assimp)
# imgui
find_package(imgui CONFIG REQUIRED)
target_link_libraries(Realtime_Cell_Collapse PRIVATE imgui::imgui)
# vulkan
find_package(Vulkan REQUIRED)
target_link_libraries(Realtime_Cell_Collapse PRIVATE Vulkan::Vulkan)

# output
get_target_property(LL Realtime_Cell_Collapse LINK_LIBRARIES)
message(STATUS "Linked libraries: ${LL}")

# add resources
add_custom_target(CopyAssets COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_LIST_DIR}/CopyAssets.cmake)
add_dependencies(Realtime_Cell_Collapse CopyAssets)