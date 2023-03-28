cmake_minimum_required(VERSION 3.24)
project(Realtime_Cell_Collapse)

set(CMAKE_CXX_STANDARD 17)

set(VK_FOLDER "C:/VulkanSDK/1.3.239.0")

set(HEADER_FOLDER include)
set(SOURCE_FOLDER source)
set(EXTERNAL_FOLDER external)
set(LIB_FOLDER lib)

# Add files
# Add any added source/header files here
#set(HEADER_FILES
#        ${HEADER_FOLDER}/parameter.h
#        ${HEADER_FOLDER}/solver.h
#        ${HEADER_FOLDER}/dataset.h
#        ${HEADER_FOLDER}/point.h
#        ${HEADER_FOLDER}/vertex.h
#        ${HEADER_FOLDER}/util.h
#        ${HEADER_FOLDER}/exporter.h
#        ${HEADER_FOLDER}/benchmarker.h)
#message(STATUS "HEADER_FILES: ${HEADER_FILES}")
#set(SOURCE_FILES
#        ${SOURCE_FOLDER}/parameter.cpp
#        ${SOURCE_FOLDER}/solver.cpp
#        ${SOURCE_FOLDER}/dataset.cpp
#        ${SOURCE_FOLDER}/point.cpp
#        ${SOURCE_FOLDER}/vertex.cpp
#        ${SOURCE_FOLDER}/util.cpp
#        ${SOURCE_FOLDER}/exporter.cpp
#        ${SOURCE_FOLDER}/benchmarker.cpp)
#message(STATUS "SOURCE_FILES: ${SOURCE_FILES}")
# Main executable
add_executable(Realtime_Cell_Collapse ${SOURCE_FOLDER}/main.cpp ${HEADER_FILES} ${SOURCE_FILES})

# Add folders
target_include_directories(Realtime_Cell_Collapse PUBLIC ${HEADER_FOLDER})
target_include_directories(Realtime_Cell_Collapse PUBLIC ${SOURCE_FOLDER})

# Submodules
#add_subdirectory(${EXTERNAL_FOLDER}/glm)
#target_link_libraries(Realtime_Cell_Collapse PUBLIC glm)
#add_subdirectory(${EXTERNAL_FOLDER}/glfw)
#target_link_libraries(Realtime_Cell_Collapse PUBLIC glfw)

# Libraries
#set(ENV{VULKAN_SDK} ${VK_FOLDER})
#find_package(Vulkan REQUIRED)
#target_include_directories(Realtime_Cell_Collapse PUBLIC ${VK_FOLDER}/Include)
#target_link_libraries(Realtime_Cell_Collapse PUBLIC ${Vulkan_LIBRARIES})
#
#get_target_property(LL Realtime_Cell_Collapse LINK_LIBRARIES)
#message(STATUS "Linked libraries: ${LL}")

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