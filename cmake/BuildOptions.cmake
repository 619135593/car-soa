# BuildOptions.cmake - 构建选项模块
# 定义项目的构建选项和配置

# 主要功能模块开关
option(BUILD_COMMUNICATION_LAYER "Build SOME/IP communication layer" ON)
option(BUILD_WEB_API "Build Web API server" ON)
option(BUILD_TESTS "Build test programs" ON)
option(BUILD_EXAMPLES "Build example programs" OFF)
option(BUILD_DOCUMENTATION "Build documentation" OFF)

# 服务客户端开关（细粒度控制）
option(BUILD_DOOR_SERVICE "Build Door Service Client" ON)
option(BUILD_WINDOW_SERVICE "Build Window Service Client" ON)
option(BUILD_LIGHT_SERVICE "Build Light Service Client" ON)
option(BUILD_SEAT_SERVICE "Build Seat Service Client" ON)

# Web API功能开关
option(BUILD_HTTP_SERVER "Build HTTP server" ON)
option(BUILD_WEBSOCKET_SERVER "Build WebSocket server" ON)
option(BUILD_WEB_FRONTEND "Install web frontend files" ON)

# 开发和调试选项
option(ENABLE_LOGGING "Enable detailed logging" ON)
option(ENABLE_DEBUG_SYMBOLS "Enable debug symbols in release builds" OFF)
option(ENABLE_SANITIZERS "Enable address and undefined behavior sanitizers" OFF)
option(ENABLE_COVERAGE "Enable code coverage analysis" OFF)
option(ENABLE_PROFILING "Enable profiling support" OFF)

# 性能和优化选项
option(ENABLE_LTO "Enable Link Time Optimization" OFF)
option(ENABLE_FAST_MATH "Enable fast math optimizations" OFF)
option(ENABLE_NATIVE_ARCH "Optimize for native architecture" OFF)

# 安全选项
option(ENABLE_HARDENING "Enable security hardening flags" ON)
option(ENABLE_STACK_PROTECTION "Enable stack protection" ON)
option(ENABLE_FORTIFY_SOURCE "Enable FORTIFY_SOURCE" ON)

# 静态分析和代码质量
option(ENABLE_CLANG_TIDY "Enable clang-tidy static analysis" OFF)
option(ENABLE_CPPCHECK "Enable cppcheck static analysis" OFF)
option(ENABLE_IWYU "Enable include-what-you-use" OFF)

# 安装和打包选项
option(INSTALL_DEVELOPMENT_FILES "Install headers and development files" ON)
option(CREATE_PACKAGE "Create installation package" OFF)

# 验证构建选项的一致性
function(validate_build_options)
    # 如果禁用通信层，则禁用所有服务客户端
    if(NOT BUILD_COMMUNICATION_LAYER)
        set(BUILD_DOOR_SERVICE OFF PARENT_SCOPE)
        set(BUILD_WINDOW_SERVICE OFF PARENT_SCOPE)
        set(BUILD_LIGHT_SERVICE OFF PARENT_SCOPE)
        set(BUILD_SEAT_SERVICE OFF PARENT_SCOPE)
        message(STATUS "Communication layer disabled, disabling all service clients")
    endif()
    
    # 如果禁用Web API，则禁用相关功能
    if(NOT BUILD_WEB_API)
        set(BUILD_HTTP_SERVER OFF PARENT_SCOPE)
        set(BUILD_WEBSOCKET_SERVER OFF PARENT_SCOPE)
        set(BUILD_WEB_FRONTEND OFF PARENT_SCOPE)
        message(STATUS "Web API disabled, disabling HTTP/WebSocket servers")
    endif()
    
    # 检查是否至少启用了一个服务客户端
    if(BUILD_COMMUNICATION_LAYER)
        if(NOT BUILD_DOOR_SERVICE AND NOT BUILD_WINDOW_SERVICE AND 
           NOT BUILD_LIGHT_SERVICE AND NOT BUILD_SEAT_SERVICE)
            message(WARNING "Communication layer enabled but no service clients selected")
        endif()
    endif()
    
    # 检查Web API依赖
    if(BUILD_WEB_API AND NOT BUILD_COMMUNICATION_LAYER)
        message(WARNING "Web API enabled but communication layer disabled - Web API may not function properly")
    endif()
endfunction()

# 应用安全加固选项
function(apply_hardening_flags target_name)
    if(NOT ENABLE_HARDENING)
        return()
    endif()
    
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        # 栈保护
        if(ENABLE_STACK_PROTECTION)
            target_compile_options(${target_name} PRIVATE -fstack-protector-strong)
        endif()
        
        # FORTIFY_SOURCE
        if(ENABLE_FORTIFY_SOURCE)
            target_compile_definitions(${target_name} PRIVATE _FORTIFY_SOURCE=2)
        endif()
        
        # 其他安全标志
        target_compile_options(${target_name} PRIVATE
            -fPIE
            -fPIC
            -Wformat
            -Wformat-security
        )
        
        target_link_options(${target_name} PRIVATE
            -pie
            -Wl,-z,relro
            -Wl,-z,now
            -Wl,-z,noexecstack
        )
        
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        # MSVC安全选项
        target_compile_options(${target_name} PRIVATE
            /GS  # 缓冲区安全检查
            /guard:cf  # 控制流保护
        )
        
        target_link_options(${target_name} PRIVATE
            /DYNAMICBASE  # ASLR
            /NXCOMPAT     # DEP
        )
    endif()
endfunction()

# 应用性能优化选项
function(apply_performance_options target_name)
    if(CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
        # Link Time Optimization
        if(ENABLE_LTO)
            set_property(TARGET ${target_name} PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
        endif()
        
        # 快速数学
        if(ENABLE_FAST_MATH AND (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang"))
            target_compile_options(${target_name} PRIVATE -ffast-math)
        endif()
        
        # 本地架构优化
        if(ENABLE_NATIVE_ARCH AND (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang"))
            target_compile_options(${target_name} PRIVATE -march=native -mtune=native)
        endif()
    endif()
endfunction()

# 应用调试和分析选项
function(apply_debug_options target_name)
    # 调试符号
    if(ENABLE_DEBUG_SYMBOLS AND CMAKE_BUILD_TYPE STREQUAL "Release")
        if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
            target_compile_options(${target_name} PRIVATE -g)
        elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
            target_compile_options(${target_name} PRIVATE /Zi)
            target_link_options(${target_name} PRIVATE /DEBUG)
        endif()
    endif()
    
    # 地址和未定义行为检测器
    if(ENABLE_SANITIZERS AND CMAKE_BUILD_TYPE STREQUAL "Debug")
        if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
            target_compile_options(${target_name} PRIVATE
                -fsanitize=address
                -fsanitize=undefined
                -fno-omit-frame-pointer
            )
            target_link_options(${target_name} PRIVATE
                -fsanitize=address
                -fsanitize=undefined
            )
        endif()
    endif()
    
    # 代码覆盖率
    if(ENABLE_COVERAGE AND CMAKE_BUILD_TYPE STREQUAL "Debug")
        if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
            target_compile_options(${target_name} PRIVATE --coverage)
            target_link_options(${target_name} PRIVATE --coverage)
        elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
            target_compile_options(${target_name} PRIVATE -fprofile-instr-generate -fcoverage-mapping)
            target_link_options(${target_name} PRIVATE -fprofile-instr-generate -fcoverage-mapping)
        endif()
    endif()
    
    # 性能分析
    if(ENABLE_PROFILING)
        if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
            target_compile_options(${target_name} PRIVATE -pg)
            target_link_options(${target_name} PRIVATE -pg)
        endif()
    endif()
endfunction()

# 设置日志级别
function(configure_logging target_name)
    if(ENABLE_LOGGING)
        if(CMAKE_BUILD_TYPE STREQUAL "Debug")
            target_compile_definitions(${target_name} PRIVATE LOG_LEVEL=DEBUG)
        else()
            target_compile_definitions(${target_name} PRIVATE LOG_LEVEL=INFO)
        endif()
    else()
        target_compile_definitions(${target_name} PRIVATE LOG_LEVEL=ERROR)
    endif()
endfunction()

# 创建通用构建选项目标
if(NOT TARGET body_controller::build_options)
    add_library(body_controller::build_options INTERFACE IMPORTED)
    
    # 应用全局编译定义
    if(ENABLE_LOGGING)
        target_compile_definitions(body_controller::build_options INTERFACE ENABLE_LOGGING)
    endif()
    
    if(BUILD_WEB_API)
        target_compile_definitions(body_controller::build_options INTERFACE BUILD_WEB_API)
    endif()
    
    if(BUILD_COMMUNICATION_LAYER)
        target_compile_definitions(body_controller::build_options INTERFACE BUILD_COMMUNICATION_LAYER)
    endif()
endif()

# 打印构建选项摘要
function(print_build_options)
    message(STATUS "")
    message(STATUS "=== Build Options ===")
    message(STATUS "Communication Layer: ${BUILD_COMMUNICATION_LAYER}")
    message(STATUS "  - Door Service: ${BUILD_DOOR_SERVICE}")
    message(STATUS "  - Window Service: ${BUILD_WINDOW_SERVICE}")
    message(STATUS "  - Light Service: ${BUILD_LIGHT_SERVICE}")
    message(STATUS "  - Seat Service: ${BUILD_SEAT_SERVICE}")
    message(STATUS "Web API: ${BUILD_WEB_API}")
    message(STATUS "  - HTTP Server: ${BUILD_HTTP_SERVER}")
    message(STATUS "  - WebSocket Server: ${BUILD_WEBSOCKET_SERVER}")
    message(STATUS "  - Web Frontend: ${BUILD_WEB_FRONTEND}")
    message(STATUS "Tests: ${BUILD_TESTS}")
    message(STATUS "Examples: ${BUILD_EXAMPLES}")
    message(STATUS "Documentation: ${BUILD_DOCUMENTATION}")
    message(STATUS "")
    message(STATUS "Development Options:")
    message(STATUS "  - Logging: ${ENABLE_LOGGING}")
    message(STATUS "  - Debug Symbols: ${ENABLE_DEBUG_SYMBOLS}")
    message(STATUS "  - Sanitizers: ${ENABLE_SANITIZERS}")
    message(STATUS "  - Coverage: ${ENABLE_COVERAGE}")
    message(STATUS "  - Hardening: ${ENABLE_HARDENING}")
    message(STATUS "  - LTO: ${ENABLE_LTO}")
    message(STATUS "=====================")
    message(STATUS "")
endfunction()

# 标记构建选项已加载
set(BODY_CONTROLLER_BUILD_OPTIONS_LOADED TRUE)
