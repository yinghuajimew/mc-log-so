cmake_minimum_required(VERSION 3.22)
project(mc_logger)

include(FetchContent)

# 自动下载 bytehook (锁定 v1.0.9 稳定版本，避开 main 分支新加入的 shadowhook 强依赖)
FetchContent_Declare(
    bytehook
    GIT_REPOSITORY https://github.com/bytedance/bhook.git
    GIT_TAG        v1.0.9
)
FetchContent_MakeAvailable(bytehook)

# ----------------------------------------------------
# 手动将 bytehook 源码编译为一个静态库 (STATIC)
# ----------------------------------------------------
# 收集 bhook 所有的 C 和 汇编 源码文件
file(GLOB BYTEHOOK_SRC 
    "${bytehook_SOURCE_DIR}/bytehook/src/main/cpp/*.c"
    "${bytehook_SOURCE_DIR}/bytehook/src/main/cpp/*.S"
)

# 创建 bytehook 目标
add_library(bytehook STATIC ${BYTEHOOK_SRC})

# 配置 bytehook 暴露自身的公共头文件 (PUBLIC)，并包含所需的内部私有头文件 (PRIVATE)
target_include_directories(bytehook PUBLIC 
    ${bytehook_SOURCE_DIR}/bytehook/src/main/cpp/include
)
target_include_directories(bytehook PRIVATE 
    ${bytehook_SOURCE_DIR}/bytehook/src/main/cpp/third_party/bsd
    ${bytehook_SOURCE_DIR}/bytehook/src/main/cpp/third_party/lss
)
# bytehook 自身需要链接 NDK 的 dl 和 log 库
target_link_libraries(bytehook dl log)


# ----------------------------------------------------
# 编译你自己的目标 mc_logger
# ----------------------------------------------------
add_library(mc_logger SHARED src/logger.cpp)

# 依赖并链接 bytehook 静态库
target_link_libraries(mc_logger
    bytehook
    log
    android
)
