# 指定源文件
set(SOURCE_FILES main.cpp)

# 生成可执行文件
add_executable(sb-downloader ${SOURCE_FILES})

# 链接Boost库
target_link_libraries(sb-downloader ${Boost_LIBRARIES})

# 链接ssl库和crypto库
target_link_libraries(sb-downloader ${Boost_LIBRARIES} ssl crypto)