# esp32_iot_node 顶层工程文件
cmake_minimum_required(VERSION 3.16)

# 嵌套组件目录：drivers 和 platforms 下的每个子目录都是一个独立组件
set(EXTRA_COMPONENT_DIRS components/drivers components/platforms)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(esp32_iot_node)
