# src/web_api/CMakeLists.txt - Web API模块
# REST服务器、WebSocket服务器和相关组件

# 检查是否启用Web API
if(NOT BUILD_WEB_API)
    message(STATUS "Web API disabled, skipping...")
    return()
endif()

# 检查Web API依赖
if(NOT HTTPLIB_AVAILABLE OR NOT JSON_AVAILABLE)
    message(FATAL_ERROR "Web API dependencies not available. Please check cpp-httplib and nlohmann/json.")
endif()

# Web API源文件
set(WEB_API_SOURCES "")

# 根据构建选项添加源文件
if(BUILD_HTTP_SERVER)
    list(APPEND WEB_API_SOURCES http_server.cpp)
    message(STATUS "Including HTTP Server")
endif()

if(BUILD_WEBSOCKET_SERVER)
    list(APPEND WEB_API_SOURCES websocket_server.cpp)
    message(STATUS "Including WebSocket Server")
endif()

# 总是包含的核心文件
list(APPEND WEB_API_SOURCES
    api_handlers.cpp
    json_converter.cpp
)

# 创建Web API静态库
add_library(body_controller_web_api STATIC
    ${WEB_API_SOURCES}
)

# 设置库的别名
add_library(body_controller::web_api ALIAS body_controller_web_api)

# 设置目标属性
set_target_properties(body_controller_web_api PROPERTIES
    OUTPUT_NAME "body_controller_web_api"
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    POSITION_INDEPENDENT_CODE ON
)

# 链接依赖
target_link_libraries(body_controller_web_api
    PUBLIC
        body_controller::communication
        body_controller::dependencies
        body_controller::compiler_settings
        body_controller::build_options
    PRIVATE
        body_controller::web_dependencies
)

# 设置包含目录
target_include_directories(body_controller_web_api
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# 应用编译器设置
apply_compiler_settings(body_controller_web_api)
apply_hardening_flags(body_controller_web_api)
apply_performance_options(body_controller_web_api)
apply_debug_options(body_controller_web_api)
configure_logging(body_controller_web_api)

# 设置编译定义
target_compile_definitions(body_controller_web_api
    PRIVATE
        BODY_CONTROLLER_WEB_API_LIBRARY
    PUBLIC
        $<$<BOOL:${BUILD_HTTP_SERVER}>:BUILD_HTTP_SERVER>
        $<$<BOOL:${BUILD_WEBSOCKET_SERVER}>:BUILD_WEBSOCKET_SERVER>
        $<$<BOOL:${BUILD_WEB_FRONTEND}>:BUILD_WEB_FRONTEND>
)

# 创建Web服务器可执行文件
if(BUILD_HTTP_SERVER OR BUILD_WEBSOCKET_SERVER)
    add_executable(body_controller_web_server
        ${CMAKE_SOURCE_DIR}/src/main_web_server.cpp
    )
    
    # 设置可执行文件属性
    set_target_properties(body_controller_web_server PROPERTIES
        OUTPUT_NAME "body_controller_web_server"
        VERSION ${PROJECT_VERSION}
    )
    
    # 链接库
    target_link_libraries(body_controller_web_server
        PRIVATE
            body_controller::web_api
            body_controller::communication
            body_controller::dependencies
            body_controller::web_dependencies
            body_controller::compiler_settings
            body_controller::build_options
    )
    
    # 应用编译器设置
    apply_compiler_settings(body_controller_web_server)
    apply_hardening_flags(body_controller_web_server)
    apply_performance_options(body_controller_web_server)
    apply_debug_options(body_controller_web_server)
    configure_logging(body_controller_web_server)
    
    # 设置运行时路径
    if(UNIX)
        set_target_properties(body_controller_web_server PROPERTIES
            INSTALL_RPATH_USE_LINK_PATH TRUE
            INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib"
        )
    endif()
endif()

# 创建单独的组件目标（可选）
if(BUILD_HTTP_SERVER)
    add_library(http_server_component OBJECT http_server.cpp)
    target_link_libraries(http_server_component PRIVATE 
        body_controller::dependencies
        body_controller::web_dependencies
    )
    apply_compiler_settings(http_server_component)
endif()

if(BUILD_WEBSOCKET_SERVER)
    add_library(websocket_server_component OBJECT websocket_server.cpp)
    target_link_libraries(websocket_server_component PRIVATE 
        body_controller::dependencies
        body_controller::web_dependencies
    )
    apply_compiler_settings(websocket_server_component)
endif()

# 安装规则
if(INSTALL_DEVELOPMENT_FILES)
    # 安装库文件
    install(TARGETS body_controller_web_api
        EXPORT body_controller_web_api_targets
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin
        INCLUDES DESTINATION include
    )
    
    # 安装可执行文件
    if(TARGET body_controller_web_server)
        install(TARGETS body_controller_web_server
            RUNTIME DESTINATION bin
        )
    endif()
    
    # 安装头文件
    install(DIRECTORY ${CMAKE_SOURCE_DIR}/include/web_api
        DESTINATION include
        FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp"
    )
    
    # 导出目标
    install(EXPORT body_controller_web_api_targets
        FILE body_controller_web_api_targets.cmake
        NAMESPACE body_controller::
        DESTINATION lib/cmake/body_controller
    )
endif()

# 创建启动脚本
if(TARGET body_controller_web_server)
    # Linux启动脚本
    if(UNIX)
        configure_file(
            ${CMAKE_SOURCE_DIR}/scripts/run_web_server.sh.in
            ${CMAKE_BINARY_DIR}/run_web_server.sh
            @ONLY
        )
        
        install(FILES ${CMAKE_BINARY_DIR}/run_web_server.sh
            DESTINATION bin
            PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                        GROUP_READ GROUP_EXECUTE
                        WORLD_READ WORLD_EXECUTE
        )
    endif()
    
    # Windows批处理脚本
    if(WIN32)
        configure_file(
            ${CMAKE_SOURCE_DIR}/scripts/run_web_server.bat.in
            ${CMAKE_BINARY_DIR}/run_web_server.bat
            @ONLY
        )
        
        install(FILES ${CMAKE_BINARY_DIR}/run_web_server.bat
            DESTINATION bin
        )
    endif()
endif()

# 添加自定义目标用于代码分析
if(ENABLE_CLANG_TIDY)
    find_program(CLANG_TIDY_EXE NAMES "clang-tidy")
    if(CLANG_TIDY_EXE)
        set_target_properties(body_controller_web_api PROPERTIES
            CXX_CLANG_TIDY "${CLANG_TIDY_EXE};-checks=-*,readability-*,performance-*,modernize-*"
        )
    endif()
endif()

if(ENABLE_CPPCHECK)
    find_program(CPPCHECK_EXE NAMES "cppcheck")
    if(CPPCHECK_EXE)
        set_target_properties(body_controller_web_api PROPERTIES
            CXX_CPPCHECK "${CPPCHECK_EXE};--enable=all;--std=c++17"
        )
    endif()
endif()

# 创建配置文件模板
configure_file(
    ${CMAKE_SOURCE_DIR}/config/web_server_config.json.in
    ${CMAKE_BINARY_DIR}/config/web_server_config.json
    @ONLY
)

# 安装配置文件
install(FILES ${CMAKE_BINARY_DIR}/config/web_server_config.json
    DESTINATION config
)

# 打印Web API构建信息
message(STATUS "")
message(STATUS "=== Web API Module ===")
message(STATUS "Sources: ${WEB_API_SOURCES}")
message(STATUS "HTTP Server: ${BUILD_HTTP_SERVER}")
message(STATUS "WebSocket Server: ${BUILD_WEBSOCKET_SERVER}")
message(STATUS "Web Frontend: ${BUILD_WEB_FRONTEND}")
message(STATUS "======================")
message(STATUS "")
