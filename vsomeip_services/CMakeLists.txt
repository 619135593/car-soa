cmake_minimum_required(VERSION 3.16)
project(body_controller_services VERSION 1.0.0)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置编译选项
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -O2")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0")

# 查找依赖包
find_package(PkgConfig REQUIRED)
find_package(Threads REQUIRED)

# 查找vsomeip
pkg_check_modules(VSOMEIP REQUIRED vsomeip3)

# 包含目录
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../include  # 使用主项目的数据结构定义
    ${VSOMEIP_INCLUDE_DIRS}
)

# 链接目录
link_directories(${VSOMEIP_LIBRARY_DIRS})

# 编译选项
add_definitions(${VSOMEIP_CFLAGS_OTHER})

# 设置输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# ============================================================================
# 公共库
# ============================================================================

# 公共源文件
set(COMMON_SOURCES
    src/common/serializer.cpp
    src/common/hardware_simulator.cpp
)

# 创建公共库
add_library(services_common STATIC ${COMMON_SOURCES})
target_link_libraries(services_common 
    ${VSOMEIP_LIBRARIES}
    Threads::Threads
)

# ============================================================================
# 服务实现库
# ============================================================================

# 服务源文件
set(SERVICE_SOURCES
    src/services/door_service.cpp
    src/services/window_service.cpp
    src/services/light_service.cpp
    src/services/seat_service.cpp
    src/services/service_manager.cpp
)

# 创建服务库
add_library(body_services STATIC ${SERVICE_SOURCES})
target_link_libraries(body_services 
    services_common
    ${VSOMEIP_LIBRARIES}
    Threads::Threads
)

# ============================================================================
# 主程序
# ============================================================================

# 主程序源文件
set(MAIN_SOURCES
    src/main.cpp
)

# 创建可执行文件
add_executable(body_controller_services ${MAIN_SOURCES})
target_link_libraries(body_controller_services 
    body_services
    services_common
    ${VSOMEIP_LIBRARIES}
    Threads::Threads
)

# ============================================================================
# 安装配置
# ============================================================================

# 安装可执行文件
install(TARGETS body_controller_services
    RUNTIME DESTINATION bin
)

# 安装配置文件
install(FILES config/vsomeip_services.json
    DESTINATION config
)

# ============================================================================
# 调试信息
# ============================================================================

message(STATUS "=== Body Controller Services Build Configuration ===")
message(STATUS "Project: ${PROJECT_NAME} v${PROJECT_VERSION}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "VSOMEIP Found: ${VSOMEIP_FOUND}")
message(STATUS "VSOMEIP Version: ${VSOMEIP_VERSION}")
message(STATUS "VSOMEIP Libraries: ${VSOMEIP_LIBRARIES}")
message(STATUS "VSOMEIP Include Dirs: ${VSOMEIP_INCLUDE_DIRS}")
message(STATUS "Output Directory: ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
message(STATUS "==================================================")
