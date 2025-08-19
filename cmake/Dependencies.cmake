# Dependencies.cmake - 依赖管理模块

# ============================================================================
# 查找必需的依赖
# ============================================================================

# 查找线程库
find_package(Threads REQUIRED)

# 查找vsomeip库
find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
    pkg_check_modules(VSOMEIP QUIET vsomeip3)
endif()

if(NOT VSOMEIP_FOUND)
    # 手动查找vsomeip
    find_path(VSOMEIP_INCLUDE_DIR 
        NAMES vsomeip/vsomeip.hpp
        PATHS /usr/include /usr/local/include
    )
    
    find_library(VSOMEIP_LIBRARIES 
        NAMES vsomeip3 vsomeip
        PATHS /usr/lib /usr/local/lib
    )
    
    if(VSOMEIP_INCLUDE_DIR AND VSOMEIP_LIBRARIES)
        set(VSOMEIP_FOUND TRUE)
        set(VSOMEIP_INCLUDE_DIRS ${VSOMEIP_INCLUDE_DIR})
    endif()
endif()

if(NOT VSOMEIP_FOUND)
    message(FATAL_ERROR "vsomeip library not found. Please install vsomeip3-dev package or build from source.")
endif()

message(STATUS "Found vsomeip include dir: ${VSOMEIP_INCLUDE_DIRS}")
message(STATUS "Found vsomeip libraries: ${VSOMEIP_LIBRARIES}")

# ============================================================================
# 查找可选依赖
# ============================================================================

# 查找Boost库（Linux下可能需要）
if(UNIX AND NOT APPLE)
    find_package(Boost COMPONENTS system filesystem thread QUIET)
    if(Boost_FOUND)
        message(STATUS "Found Boost: ${Boost_VERSION}")
    endif()
endif()

# ============================================================================
# Web服务器依赖（仅在需要时获取）
# ============================================================================

if(BUILD_WEB_SERVER)
    include(FetchContent)
    
    # 获取cpp-httplib
    FetchContent_Declare(
        httplib
        GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
        GIT_TAG v0.14.0
    )
    
    # 获取nlohmann/json
    FetchContent_Declare(
        json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG v3.11.2
    )
    
    FetchContent_MakeAvailable(httplib json)
    
    message(STATUS "Web server dependencies fetched successfully")
endif()

# ============================================================================
# 创建导入目标（如果需要）
# ============================================================================

if(VSOMEIP_FOUND AND NOT TARGET vsomeip::vsomeip)
    add_library(vsomeip::vsomeip INTERFACE IMPORTED)
    target_include_directories(vsomeip::vsomeip INTERFACE ${VSOMEIP_INCLUDE_DIRS})
    target_link_libraries(vsomeip::vsomeip INTERFACE ${VSOMEIP_LIBRARIES})
endif()

# ============================================================================
# 平台特定设置
# ============================================================================

if(WIN32)
    # Windows特定设置
    set(PLATFORM_LIBS ws2_32 wsock32)
elseif(UNIX)
    # Unix/Linux特定设置
    set(PLATFORM_LIBS ${CMAKE_DL_LIBS})
    if(Boost_FOUND)
        list(APPEND PLATFORM_LIBS ${Boost_LIBRARIES})
    endif()
endif()

# ============================================================================
# 导出变量供子模块使用
# ============================================================================

set(BODY_CONTROLLER_DEPENDENCIES
    Threads::Threads
    vsomeip::vsomeip
    ${PLATFORM_LIBS}
    PARENT_SCOPE
)

if(BUILD_WEB_SERVER)
    set(WEB_SERVER_DEPENDENCIES
        httplib
        nlohmann_json
        PARENT_SCOPE
    )
endif()
