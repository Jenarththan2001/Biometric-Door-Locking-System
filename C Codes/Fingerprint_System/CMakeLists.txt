cmake_minimum_required(VERSION 3.16)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(OLED_Test)

set(EXTRA_COMPONENT_DIRS components/oled_driver)
