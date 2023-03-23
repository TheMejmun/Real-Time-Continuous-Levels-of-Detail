cmake_minimum_required(VERSION 3.24)
project(Realtime_Cell_Collapse)

set(CMAKE_CXX_STANDARD 17)

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

# OGDF
#add_subdirectory(${EXTERN_FOLDER}/ogdf)
#target_link_libraries(Realtime_Cell_Collapse PUBLIC OGDF)
