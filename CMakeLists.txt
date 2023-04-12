cmake_minimum_required(VERSION 3.5)
project(people_msgs_utils_vis)

add_definitions(-std=c++17)

find_package(catkin REQUIRED
    COMPONENTS
        people_msgs_utils
        visualization_msgs
)

include_directories(
    ${catkin_INCLUDE_DIRS}
    ${people_msgs_utils_INCLUDE_DIRS}
)

catkin_package(
    CATKIN_DEPENDS
        people_msgs_utils
)

## Library
add_executable(visualization_node
    src/visualization_node.cpp
)
target_link_libraries(visualization_node
    ${catkin_LIBRARIES}
    ${people_msgs_utils_LIBRARIES}
)

## Install
install(TARGETS visualization_node
    ARCHIVE DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
    LIBRARY DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
)
