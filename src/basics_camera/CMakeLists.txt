cmake_minimum_required(VERSION 2.8.3)
project(basics_camera)

find_package(catkin REQUIRED COMPONENTS
  cv_bridge
  image_transport
  roscpp
  rospy
  sensor_msgs
  std_msgs
)

find_package(Boost REQUIRED COMPONENTS system)

find_package(OpenCV REQUIRED COMPONENTS core imgproc highgui objdetect)

catkin_package(
  CATKIN_DEPENDS roscpp rospy std_msgs

)


include_directories(
  ${catkin_INCLUDE_DIRS}
)


add_executable(basics_camera_node src/basics_camera.cpp)
target_link_libraries(basics_camera_node
   ${catkin_LIBRARIES}
   ${OpenCV_LIBRARIES}
 )