# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.16)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
set(COMPONENTS main VL53L5CX)
# "Trim" the build. Include the minimal set of components, main, and anything it depends on.
project(VL53L5CX_SENSOR)