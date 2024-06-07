# 查找Google Test库
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

# 指定测试源文件
set(TEST_SOURCE_FILES test_main.cpp ...)

# 生成测试可执行文件
add_executable(sb-downloader ${TEST_SOURCE_FILES})

# 链接Google Test和Boost库
target_link_libraries(sb-downloader ${GTEST_LIBRARIES} ${Boost_LIBRARIES} pthread)