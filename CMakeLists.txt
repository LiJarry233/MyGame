# CMake 最低版本要求
cmake_minimum_required(VERSION 3.15)

# 项目名称和 C++ 标准
project(MyRaylibGame VERSION 1.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Raylib 库查找（假设已安装并可用）
find_package(raylib REQUIRED)

# 所有源文件
set(SOURCES
    main.cpp
    Ball.cpp
    Paddle.cpp
    Brick.cpp
)

# 生成可执行文件
add_executable(MyRaylibGame ${SOURCES})

# 链接 Raylib
target_link_libraries(MyRaylibGame PRIVATE raylib)

# 如果使用 Windows/MSVC，可以添加一些编译选项（可选）
if(MSVC)
    # 多字节字符集
    add_compile_options(/utf-8)
endif()