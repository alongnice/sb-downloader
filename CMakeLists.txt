#cmake 最低版本
cmake_minimum_required(VERSION 3.10)
# 项目名称 & 版本 & 描述
project(sb-downloader VERSION 1.0 DESCRIPTION "Shared bandwidth downloader")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# 查找Boost库
find_package(Boost 1.67.0 REQUIRED COMPONENTS filesystem thread)
if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
endif()

# 设置编译后的二进制文件存放位置
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
# 设置库文件存放位置
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

# 包含头文件目录
include_directories(${PROJECT_SOURCE_DIR}/include)


# 添加src子目录
add_subdirectory(src)

# 添加test子目录
#enable_testing()
#add_subdirectory(test)