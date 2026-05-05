# CMake 最低版本要求
cmake_minimum_required(VERSION 3.15)

# 项目名称和 C++ 标准
project(MyRaylibGame VERSION 1.0 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# 设置raylib路径
set(RAYLIB_DIR "${CMAKE_SOURCE_DIR}/raylib/raylib-5.0_win64_msvc16")

# 包含目录
include_directories("${RAYLIB_DIR}/include")
include_directories("${CMAKE_SOURCE_DIR}/include")
include_directories("${CMAKE_SOURCE_DIR}/enet/include")

# 链接目录
link_directories("${RAYLIB_DIR}/lib")

# 可执行文件
add_executable(MyRaylibGame main.cpp NetworkManager.cpp enet/src/enet.c)

# 添加测试
add_executable(Test Test.cpp)

# 链接库
target_link_libraries(MyRaylibGame PRIVATE raylib winmm ws2_32)
target_link_libraries(Test PRIVATE raylib winmm)

# Windows 编码（可选）
if(MSVC)
    add_compile_options(/utf-8)
endif()