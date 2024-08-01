cmake_minimum_required(VERSION 3.22.1)
project(server CXX)

# 定义变量
set(BUILD_PATH ${PROJECT_SOURCE_DIR}/bin)                       # 二进制文件输出路径
set(LINK_LIB pthread event jsoncpp mysqlcppconn cryptopp)       # 链接动态库列表

# 生成可执行文件
# 搜索源文件
aux_source_directory(${PROJECT_SOURCE_DIR}/src SRC_LIST)
# 包含头文件
include_directories(${PROJECT_SOURCE_DIR}/src/include)
# 设置输出路径
set(EXECUTABLE_OUTPUT_PATH ${BUILD_PATH})
# 添加目标
add_executable(server ${SRC_LIST})

# 链接第三方库
# 链接库依赖文件
link_directories(${PROJECT_SOURCE_DIR}/lib/)
# 链接系统动态库
target_link_libraries(server ${LINK_LIB})

# 添加 DEBUG 宏
#  add_definitions(-D__DEBUG)
