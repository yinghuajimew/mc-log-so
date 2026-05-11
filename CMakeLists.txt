cmake_minimum_required(VERSION 3.22)
project(mc_logger)

include(FetchContent)

# 自动下载 bytehook
FetchContent_Declare(
    bytehook
    GIT_REPOSITORY https://github.com/bytedance/bhook.git
    GIT_TAG        main
)
FetchContent_MakeAvailable(bytehook)

add_library(mc_logger SHARED src/logger.cpp)

# 手动添加 bytehook 的头文件搜索路径，确保可以找到 bytehook.h
target_include_directories(mc_logger PRIVATE ${bytehook_SOURCE_DIR}/bytehook/src/main/cpp/include)

target_link_libraries(mc_logger
    bytehook
    log
    android
)