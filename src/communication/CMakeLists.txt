# src/communication/CMakeLists.txt - 通信层模块
# SOME/IP客户端库和相关组件

# 检查是否启用通信层
if(NOT BUILD_COMMUNICATION_LAYER)
    message(STATUS "Communication layer disabled, skipping...")
    return()
endif()

# 通信层源文件
set(COMMUNICATION_SOURCES
    someip_client.cpp
    serialization.cpp
)

# 服务客户端源文件（根据构建选项添加）
set(SERVICE_CLIENT_SOURCES "")

if(BUILD_DOOR_SERVICE)
    list(APPEND SERVICE_CLIENT_SOURCES door_service_client.cpp)
    message(STATUS "Including Door Service Client")
endif()

if(BUILD_WINDOW_SERVICE)
    list(APPEND SERVICE_CLIENT_SOURCES window_service_client.cpp)
    message(STATUS "Including Window Service Client")
endif()

if(BUILD_LIGHT_SERVICE)
    list(APPEND SERVICE_CLIENT_SOURCES light_service_client.cpp)
    message(STATUS "Including Light Service Client")
endif()

if(BUILD_SEAT_SERVICE)
    list(APPEND SERVICE_CLIENT_SOURCES seat_service_client.cpp)
    message(STATUS "Including Seat Service Client")
endif()

# 创建通信层静态库
add_library(body_controller_communication STATIC
    ${COMMUNICATION_SOURCES}
    ${SERVICE_CLIENT_SOURCES}
)

# 设置库的别名
add_library(body_controller::communication ALIAS body_controller_communication)

# 设置目标属性
set_target_properties(body_controller_communication PROPERTIES
    OUTPUT_NAME "body_controller_communication"
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    POSITION_INDEPENDENT_CODE ON
)

# 链接依赖
target_link_libraries(body_controller_communication
    PUBLIC
        body_controller::dependencies
        body_controller::compiler_settings
        body_controller::build_options
    PRIVATE
        ${VSOMEIP_LIBRARIES}
)

# 设置包含目录
target_include_directories(body_controller_communication
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${VSOMEIP_INCLUDE_DIRS}
)

# 应用编译器设置
apply_compiler_settings(body_controller_communication)
apply_hardening_flags(body_controller_communication)
apply_performance_options(body_controller_communication)
apply_debug_options(body_controller_communication)
configure_logging(body_controller_communication)

# 设置编译定义
target_compile_definitions(body_controller_communication
    PRIVATE
        BODY_CONTROLLER_COMMUNICATION_LIBRARY
    PUBLIC
        $<$<BOOL:${BUILD_DOOR_SERVICE}>:BUILD_DOOR_SERVICE>
        $<$<BOOL:${BUILD_WINDOW_SERVICE}>:BUILD_WINDOW_SERVICE>
        $<$<BOOL:${BUILD_LIGHT_SERVICE}>:BUILD_LIGHT_SERVICE>
        $<$<BOOL:${BUILD_SEAT_SERVICE}>:BUILD_SEAT_SERVICE>
)

# 创建服务客户端的单独目标（可选，用于细粒度控制）
if(BUILD_DOOR_SERVICE)
    add_library(door_service_client OBJECT door_service_client.cpp)
    target_link_libraries(door_service_client PRIVATE body_controller::dependencies)
    apply_compiler_settings(door_service_client)
endif()

if(BUILD_WINDOW_SERVICE)
    add_library(window_service_client OBJECT window_service_client.cpp)
    target_link_libraries(window_service_client PRIVATE body_controller::dependencies)
    apply_compiler_settings(window_service_client)
endif()

if(BUILD_LIGHT_SERVICE)
    add_library(light_service_client OBJECT light_service_client.cpp)
    target_link_libraries(light_service_client PRIVATE body_controller::dependencies)
    apply_compiler_settings(light_service_client)
endif()

if(BUILD_SEAT_SERVICE)
    add_library(seat_service_client OBJECT seat_service_client.cpp)
    target_link_libraries(seat_service_client PRIVATE body_controller::dependencies)
    apply_compiler_settings(seat_service_client)
endif()

# 安装规则
if(INSTALL_DEVELOPMENT_FILES)
    # 安装库文件
    install(TARGETS body_controller_communication
        EXPORT body_controller_communication_targets
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin
        INCLUDES DESTINATION include
    )
    
    # 安装头文件
    install(DIRECTORY ${CMAKE_SOURCE_DIR}/include/communication
        DESTINATION include
        FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp"
    )
    
    install(DIRECTORY ${CMAKE_SOURCE_DIR}/include/application
        DESTINATION include
        FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp"
    )
    
    # 导出目标
    install(EXPORT body_controller_communication_targets
        FILE body_controller_communication_targets.cmake
        NAMESPACE body_controller::
        DESTINATION lib/cmake/body_controller
    )
endif()

# 创建配置文件
if(INSTALL_DEVELOPMENT_FILES)
    include(CMakePackageConfigHelpers)
    
    configure_package_config_file(
        ${CMAKE_SOURCE_DIR}/cmake/body_controller_communication_config.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/body_controller_communication_config.cmake
        INSTALL_DESTINATION lib/cmake/body_controller
    )
    
    write_basic_package_version_file(
        ${CMAKE_CURRENT_BINARY_DIR}/body_controller_communication_config_version.cmake
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion
    )
    
    install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/body_controller_communication_config.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/body_controller_communication_config_version.cmake
        DESTINATION lib/cmake/body_controller
    )
endif()

# 添加自定义目标用于代码分析
if(ENABLE_CLANG_TIDY)
    find_program(CLANG_TIDY_EXE NAMES "clang-tidy")
    if(CLANG_TIDY_EXE)
        set_target_properties(body_controller_communication PROPERTIES
            CXX_CLANG_TIDY "${CLANG_TIDY_EXE};-checks=-*,readability-*,performance-*,modernize-*"
        )
    endif()
endif()

if(ENABLE_CPPCHECK)
    find_program(CPPCHECK_EXE NAMES "cppcheck")
    if(CPPCHECK_EXE)
        set_target_properties(body_controller_communication PROPERTIES
            CXX_CPPCHECK "${CPPCHECK_EXE};--enable=all;--std=c++17"
        )
    endif()
endif()

# 打印通信层构建信息
message(STATUS "")
message(STATUS "=== Communication Layer ===")
message(STATUS "Sources: ${COMMUNICATION_SOURCES}")
message(STATUS "Service Clients: ${SERVICE_CLIENT_SOURCES}")
message(STATUS "vsomeip Libraries: ${VSOMEIP_LIBRARIES}")
message(STATUS "===========================")
message(STATUS "")
