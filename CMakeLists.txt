
cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

add_definitions(-std=c++11)

set(CXX_FLAGS "-Wall")
set(CMAKE_CXX_FLAGS, "${CXX_FLAGS}")

project(camera_fusion)

find_package(OpenCV REQUIRED)

include_directories(${OpenCV_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR}/src)
link_directories(${OpenCV_LIBRARY_DIRS})
add_definitions(${OpenCV_DEFINITIONS})

# Executable for create matrix exercise
add_executable (3D_object_tracking src/camFusion_Student.cpp src/FinalProject_Camera.cpp src/lidarData.cpp src/matching2D_Student.cpp src/objectDetection2D.cpp)
target_link_libraries (3D_object_tracking ${OpenCV_LIBRARIES})

# Executable for Stage 0
add_executable (Stage00_RawCamera
    stage_code/00_raw_camera/Stage00_RawCamera.cpp
)

target_link_libraries(
    Stage00_RawCamera
    ${OpenCV_LIBRARIES}
)

# Executable for Stage 1 - YOLO all vehicle detections
add_executable (Stage01_YOLO stage_code/01_yolo_all_vehicles/Stage01_YOLO.cpp src/objectDetection2D.cpp)
target_link_libraries (Stage01_YOLO ${OpenCV_LIBRARIES})

# Executable for Stage 2 - LiDAR detection and spatial filtering
add_executable (Stage02_LiDAR stage_code/02_lidar_detection/Stage02_LiDAR.cpp src/lidarData.cpp)
target_link_libraries (Stage02_LiDAR ${OpenCV_LIBRARIES})

# Executable for Stage 3
add_executable (Stage03_LiDARProjection
    stage_code/03_lidar_projection/Stage03_LiDARProjection.cpp
    src/lidarData.cpp
)

target_link_libraries(
    Stage03_LiDARProjection
    ${OpenCV_LIBRARIES}
)

# Executable for Stage 4
add_executable (Stage04_SensorFusion
    stage_code/04_sensor_fusion/Stage04_SensorFusion.cpp
    src/lidarData.cpp
    src/objectDetection2D.cpp
)

target_link_libraries(
    Stage04_SensorFusion
    ${OpenCV_LIBRARIES}
)

# Executable for Stage 5
add_executable (Stage05_TargetVehicle
    stage_code/05_target_vehicle/Stage05_TargetVehicle.cpp
    src/lidarData.cpp
    src/objectDetection2D.cpp
)

target_link_libraries(
    Stage05_TargetVehicle
    ${OpenCV_LIBRARIES}
)