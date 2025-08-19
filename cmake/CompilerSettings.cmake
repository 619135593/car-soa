# CompilerSettings.cmake - 编译器设置模块
# 统一管理编译器相关的设置和标志

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 设置C标准
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# 启用导出编译命令（用于IDE和静态分析工具）
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# 设置默认构建类型
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "RelWithDebInfo" "MinSizeRel")
endif()

# 编译器特定设置
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    # GCC特定设置
    set(COMPILER_NAME "GCC")
    set(COMPILER_VERSION ${CMAKE_CXX_COMPILER_VERSION})
    
    # GCC警告标志
    set(WARNING_FLAGS
        -Wall
        -Wextra
        -Wpedantic
        -Wcast-align
        -Wcast-qual
        -Wctor-dtor-privacy
        -Wdisabled-optimization
        -Wformat=2
        -Winit-self
        -Wlogical-op
        -Wmissing-declarations
        -Wmissing-include-dirs
        -Wnoexcept
        -Wold-style-cast
        -Woverloaded-virtual
        -Wredundant-decls
        -Wshadow
        -Wsign-conversion
        -Wsign-promo
        -Wstrict-null-sentinel
        -Wstrict-overflow=5
        -Wswitch-default
        -Wundef
        -Wno-unused-parameter
    )
    
    # GCC优化标志
    set(OPTIMIZATION_FLAGS_DEBUG -Og -g3)
    set(OPTIMIZATION_FLAGS_RELEASE -O3 -DNDEBUG)
    
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    # Clang特定设置
    set(COMPILER_NAME "Clang")
    set(COMPILER_VERSION ${CMAKE_CXX_COMPILER_VERSION})
    
    # Clang警告标志
    set(WARNING_FLAGS
        -Wall
        -Wextra
        -Wpedantic
        -Wcast-align
        -Wcast-qual
        -Wformat=2
        -Wmissing-declarations
        -Woverlength-strings
        -Wpointer-arith
        -Wredundant-decls
        -Wshadow
        -Wunreachable-code
        -Wno-unused-parameter
    )
    
    # Clang优化标志
    set(OPTIMIZATION_FLAGS_DEBUG -Og -g)
    set(OPTIMIZATION_FLAGS_RELEASE -O3 -DNDEBUG)
    
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    # MSVC特定设置
    set(COMPILER_NAME "MSVC")
    set(COMPILER_VERSION ${CMAKE_CXX_COMPILER_VERSION})
    
    # MSVC警告标志
    set(WARNING_FLAGS
        /W4
        /WX-  # 不将警告视为错误
        /wd4251  # 禁用DLL接口警告
        /wd4275  # 禁用DLL基类警告
    )
    
    # MSVC优化标志
    set(OPTIMIZATION_FLAGS_DEBUG /Od /Zi)
    set(OPTIMIZATION_FLAGS_RELEASE /O2 /DNDEBUG)
    
    # MSVC特定定义
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    add_definitions(-DNOMINMAX)
    add_definitions(-DWIN32_LEAN_AND_MEAN)
    
else()
    # 未知编译器
    set(COMPILER_NAME "Unknown")
    set(WARNING_FLAGS "")
    set(OPTIMIZATION_FLAGS_DEBUG "")
    set(OPTIMIZATION_FLAGS_RELEASE "")
    message(WARNING "Unknown compiler: ${CMAKE_CXX_COMPILER_ID}")
endif()

# 平台特定设置
if(WIN32)
    # Windows特定设置
    add_definitions(-DWIN32)
    add_definitions(-D_WIN32_WINNT=0x0601)  # Windows 7+
    
elseif(UNIX AND NOT APPLE)
    # Linux特定设置
    add_definitions(-DLINUX)
    
    # 启用大文件支持
    add_definitions(-D_FILE_OFFSET_BITS=64)
    add_definitions(-D_LARGEFILE_SOURCE)
    
elseif(APPLE)
    # macOS特定设置
    add_definitions(-DMACOS)
endif()

# 调试/发布特定设置
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_definitions(-DDEBUG)
    add_definitions(-D_DEBUG)
else()
    add_definitions(-DNDEBUG)
endif()

# 创建编译器设置函数
function(apply_compiler_settings target_name)
    # 应用警告标志
    if(WARNING_FLAGS)
        target_compile_options(${target_name} PRIVATE ${WARNING_FLAGS})
    endif()
    
    # 应用优化标志
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        if(OPTIMIZATION_FLAGS_DEBUG)
            target_compile_options(${target_name} PRIVATE ${OPTIMIZATION_FLAGS_DEBUG})
        endif()
    else()
        if(OPTIMIZATION_FLAGS_RELEASE)
            target_compile_options(${target_name} PRIVATE ${OPTIMIZATION_FLAGS_RELEASE})
        endif()
    endif()
    
    # 设置C++标准
    target_compile_features(${target_name} PRIVATE cxx_std_17)
    
    # 设置包含目录
    target_include_directories(${target_name} PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_BINARY_DIR}
    )
endfunction()

# 创建通用编译器设置目标
if(NOT TARGET body_controller::compiler_settings)
    add_library(body_controller::compiler_settings INTERFACE IMPORTED)
    
    # 设置编译特性
    target_compile_features(body_controller::compiler_settings INTERFACE cxx_std_17)
    
    # 添加编译选项
    if(WARNING_FLAGS)
        target_compile_options(body_controller::compiler_settings INTERFACE ${WARNING_FLAGS})
    endif()
    
    # 添加构建类型特定的选项
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        if(OPTIMIZATION_FLAGS_DEBUG)
            target_compile_options(body_controller::compiler_settings INTERFACE ${OPTIMIZATION_FLAGS_DEBUG})
        endif()
    else()
        if(OPTIMIZATION_FLAGS_RELEASE)
            target_compile_options(body_controller::compiler_settings INTERFACE ${OPTIMIZATION_FLAGS_RELEASE})
        endif()
    endif()
    
    # 添加全局包含目录
    target_include_directories(body_controller::compiler_settings INTERFACE
        ${CMAKE_SOURCE_DIR}/include
    )
endif()

# 打印编译器信息
function(print_compiler_info)
    message(STATUS "")
    message(STATUS "=== Compiler Information ===")
    message(STATUS "Compiler: ${COMPILER_NAME} ${COMPILER_VERSION}")
    message(STATUS "C++ Standard: C++${CMAKE_CXX_STANDARD}")
    message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
    message(STATUS "System: ${CMAKE_SYSTEM_NAME} ${CMAKE_SYSTEM_VERSION}")
    message(STATUS "Architecture: ${CMAKE_SYSTEM_PROCESSOR}")
    if(WARNING_FLAGS)
        message(STATUS "Warning Flags: ${WARNING_FLAGS}")
    endif()
    message(STATUS "============================")
    message(STATUS "")
endfunction()

# 设置输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# 为多配置生成器设置输出目录
foreach(CONFIG ${CMAKE_CONFIGURATION_TYPES})
    string(TOUPPER ${CONFIG} CONFIG_UPPER)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${CONFIG_UPPER} ${CMAKE_BINARY_DIR}/bin)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${CONFIG_UPPER} ${CMAKE_BINARY_DIR}/lib)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${CONFIG_UPPER} ${CMAKE_BINARY_DIR}/lib)
endforeach()

# 标记编译器设置已加载
set(BODY_CONTROLLER_COMPILER_SETTINGS_LOADED TRUE)
