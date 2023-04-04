cmake_minimum_required(VERSION 3.24)
project(Realtime_Cell_Collapse)

set(CMAKE_CXX_STANDARD 17)

if (WIN32)
    set(VK_FOLDER "C:/VulkanSDK/1.3.239.0")
    message(STATUS "Windows mode")
elseif (LINUX)
    message(STATUS "Linux mode")
elseif (UNIX)
    set(VK_FOLDER "$ENV{HOME}/VulkanSDK/1.3.239.0")
    message(STATUS "Unix mode")
endif ()

set(HEADER_FOLDER include)
set(SOURCE_FOLDER source)
set(EXTERNAL_FOLDER external)
set(LIB_FOLDER lib)

# Add files
# Add any added source/header files here
set(HEADER_FILES
        ${HEADER_FOLDER}/application.h)
message(STATUS "HEADER_FILES: ${HEADER_FILES}")
set(SOURCE_FILES
        ${SOURCE_FOLDER}/application.cpp)
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

# output
get_target_property(LL Realtime_Cell_Collapse LINK_LIBRARIES)
message(STATUS "Linked libraries: ${LL}")