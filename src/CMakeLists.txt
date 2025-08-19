# src/CMakeLists.txt - 源码模块总入口
# 协调各个源码子模块的构建

# 通信层模块
if(BUILD_COMMUNICATION_LAYER)
    add_subdirectory(communication)
    message(STATUS "Added communication layer module")
endif()

# Web API模块
if(BUILD_WEB_API)
    add_subdirectory(web_api)
    message(STATUS "Added web API module")
endif()

# 测试模块
if(BUILD_TESTS)
    add_subdirectory(tests)
    message(STATUS "Added tests module")
endif()

# 示例程序模块
if(BUILD_EXAMPLES)
    # 如果有示例程序目录，可以在这里添加
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/examples)
        add_subdirectory(examples)
        message(STATUS "Added examples module")
    endif()
endif()

# 打印源码模块信息
message(STATUS "")
message(STATUS "=== Source Modules ===")
message(STATUS "Communication: ${BUILD_COMMUNICATION_LAYER}")
message(STATUS "Web API: ${BUILD_WEB_API}")
message(STATUS "Tests: ${BUILD_TESTS}")
message(STATUS "Examples: ${BUILD_EXAMPLES}")
message(STATUS "======================")
message(STATUS "")
