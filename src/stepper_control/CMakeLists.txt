cmake_minimum_required(VERSION 3.0.2)
project(stepper_control)

find_package(catkin REQUIRED COMPONENTS
  roscpp
  std_msgs
)

catkin_package(
  CATKIN_DEPENDS roscpp std_msgs
)

include_directories(
  ${catkin_INCLUDE_DIRS}
)

add_executable(stepper_control_node src/stepper_control.cpp)
target_link_libraries(stepper_control_node
   ${catkin_LIBRARIES} -lwiringPi -lpthread
 )