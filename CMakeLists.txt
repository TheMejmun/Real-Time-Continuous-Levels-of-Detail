cmake_minimum_required(VERSION 3.24)
project(Realtime_Cell_Collapse)

set(CMAKE_CXX_STANDARD 20)

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
        ${HEADER_FOLDER}/timer.h
        ${HEADER_FOLDER}/renderer.h
        ${HEADER_FOLDER}/importer.h
        ${HEADER_FOLDER}/application.h
        ${HEADER_FOLDER}/input_manager.h
        ${HEADER_FOLDER}/window_manager.h
        ${HEADER_FOLDER}/colors.h
        ${HEADER_FOLDER}/vertex.h
        ${HEADER_FOLDER}/triangle.h
        ${HEADER_FOLDER}/ecs.h
        ${HEADER_FOLDER}/renderable.h
        ${HEADER_FOLDER}/printer.h
        ${HEADER_FOLDER}/vbuffer_manager.h include/queue_family_indices.h)
message(STATUS "HEADER_FILES: ${HEADER_FILES}")
set(SOURCE_FILES
        ${SOURCE_FOLDER}/importer.cpp
        ${SOURCE_FOLDER}/application.cpp
        ${SOURCE_FOLDER}/input_manager.cpp
        ${SOURCE_FOLDER}/window_manager.cpp
        ${SOURCE_FOLDER}/ecs.cpp
        ${SOURCE_FOLDER}/renderer/drawing.cpp
        ${SOURCE_FOLDER}/renderer/device.cpp
        ${SOURCE_FOLDER}/renderer/swapchain.cpp
        ${SOURCE_FOLDER}/queue_family_indices.cpp
        ${SOURCE_FOLDER}/renderer/debug.cpp
        ${SOURCE_FOLDER}/renderer/common.cpp
        ${SOURCE_FOLDER}/renderer/shaders.cpp
        ${SOURCE_FOLDER}/vbuffer_manager.cpp
        ${SOURCE_FOLDER}/colors.cpp)
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
# glm
find_package(glm CONFIG REQUIRED)
target_link_libraries(Realtime_Cell_Collapse PRIVATE glm::glm)
# vulkan
find_package(Vulkan REQUIRED)
target_link_libraries(Realtime_Cell_Collapse PRIVATE Vulkan::Vulkan)
# vulkan hpp
find_path(VULKAN_HPP_INCLUDE_DIRS "vulkan/vulkan.hpp")
target_include_directories(Realtime_Cell_Collapse PRIVATE ${VULKAN_HPP_INCLUDE_DIRS})
# imgui
find_package(imgui CONFIG REQUIRED)
target_link_libraries(Realtime_Cell_Collapse PRIVATE imgui::imgui)
# assimp
find_package(assimp CONFIG REQUIRED)
target_link_libraries(Realtime_Cell_Collapse PRIVATE assimp::assimp)

# output
get_target_property(LL Realtime_Cell_Collapse LINK_LIBRARIES)
message(STATUS "Linked libraries: ${LL}")

# add resources
add_custom_target(CopyAssets COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_LIST_DIR}/CopyAssets.cmake)
add_dependencies(Realtime_Cell_Collapse CopyAssets)