cmake_minimum_required(VERSION 3.1)
project( ObjectDetector )
set (CMAKE_CXX_STANDARD 11)
find_package( OpenCV REQUIRED )
file(GLOB ObjectDetector_SRC
    "*.h"
    "*.cpp"
)
add_executable( ObjectDetector ${ObjectDetector_SRC} )
target_link_libraries( ObjectDetector ${OpenCV_LIBS} )
