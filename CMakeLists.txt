cmake_minimum_required(VERSION 3.16)
project(BodyController VERSION 1.0.0 LANGUAGES CXX)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# 编译选项
if(MSVC)
    add_compile_options(/W4)
    add_definitions(-D_WIN32_WINNT=0x0601)  # Windows 7+
else()
    # Linux/GCC编译选项
    add_compile_options(-Wall -Wextra -Wpedantic -pthread)
    set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -DDEBUG")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
endif()

# 查找依赖包
find_package(Threads REQUIRED)
find_package(PkgConfig QUIET)

# 设置CMAKE模块路径，让CMake能找到vsomeip配置
list(APPEND CMAKE_MODULE_PATH "/usr/local/lib/cmake/vsomeip3")
list(APPEND CMAKE_PREFIX_PATH "/usr/local")

# 查找vsomeip
find_package(vsomeip3 QUIET)
if(vsomeip3_FOUND)
    message(STATUS "✅ Found vsomeip3 via find_package")
    set(VSOMEIP_LIBRARIES vsomeip3)
    set(VSOMEIP_INCLUDE_DIRS ${vsomeip3_INCLUDE_DIRS})
else()
    # 如果没有找到vsomeip3，尝试查找vsomeip
    find_package(vsomeip QUIET)
    if(vsomeip_FOUND)
        message(STATUS "✅ Found vsomeip via find_package")
        set(VSOMEIP_LIBRARIES ${vsomeip_LIBRARIES})
        set(VSOMEIP_INCLUDE_DIRS ${vsomeip_INCLUDE_DIRS})
    else()
        message(STATUS "⚠️  vsomeip not found via find_package, setting up manually...")

        # 手动设置vsomeip库和头文件路径
        set(VSOMEIP_INCLUDE_DIRS "/usr/local/include")

        # 查找vsomeip库文件
        find_library(VSOMEIP3_LIBRARY
            NAMES vsomeip3
            PATHS /usr/local/lib
            NO_DEFAULT_PATH
        )

        if(VSOMEIP3_LIBRARY)
            set(VSOMEIP_LIBRARIES ${VSOMEIP3_LIBRARY})
            message(STATUS "✅ Found vsomeip3 library: ${VSOMEIP3_LIBRARY}")
        else()
            message(FATAL_ERROR "❌ Could not find vsomeip3 library in /usr/local/lib")
        endif()

        # 检查头文件是否存在
        if(NOT EXISTS "${VSOMEIP_INCLUDE_DIRS}/vsomeip/vsomeip.hpp")
            message(FATAL_ERROR "❌ Could not find vsomeip headers in ${VSOMEIP_INCLUDE_DIRS}")
        endif()

        message(STATUS "✅ Using manual vsomeip configuration:")
        message(STATUS "    Include dirs: ${VSOMEIP_INCLUDE_DIRS}")
        message(STATUS "    Libraries: ${VSOMEIP_LIBRARIES}")
    endif()
endif()

# Linux特定的包查找
if(UNIX AND NOT APPLE)
    # 查找Boost (Linux下通常需要)
    find_package(Boost COMPONENTS system filesystem thread QUIET)
endif()

# 优先使用系统包，其次使用FetchContent兜底
# 1) nlohmann_json
set(HAVE_SYSTEM_JSON FALSE)
find_package(nlohmann_json QUIET)
if(nlohmann_json_FOUND)
    if(TARGET nlohmann_json::nlohmann_json AND NOT TARGET nlohmann_json)
        add_library(nlohmann_json ALIAS nlohmann_json::nlohmann_json)
    endif()
    set(HAVE_SYSTEM_JSON TRUE)
endif()

# 2) cpp-httplib
set(HAVE_SYSTEM_HTTPLIB FALSE)
find_package(httplib QUIET CONFIG)
if(TARGET httplib::httplib)
    if(NOT TARGET httplib)
        add_library(httplib ALIAS httplib::httplib)
    endif()
    set(HAVE_SYSTEM_HTTPLIB TRUE)
else()
    if(PkgConfig_FOUND)
        pkg_check_modules(CPP_HTTPLIB QUIET cpp-httplib)
        if(CPP_HTTPLIB_FOUND)
            if(NOT TARGET httplib)
                add_library(httplib INTERFACE)
            endif()
            target_include_directories(httplib INTERFACE ${CPP_HTTPLIB_INCLUDE_DIRS})
            target_compile_definitions(httplib INTERFACE ${CPP_HTTPLIB_CFLAGS_OTHER})
            target_link_libraries(httplib INTERFACE ${CPP_HTTPLIB_LIBRARIES})
            set(HAVE_SYSTEM_HTTPLIB TRUE)
        endif()
    endif()
endif()

# 3) 对未找到的包使用FetchContent
include(FetchContent)
if(NOT HAVE_SYSTEM_HTTPLIB)
    FetchContent_Declare(
        httplib
        GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
        GIT_TAG v0.14.0
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
    )
    FetchContent_MakeAvailable(httplib)
endif()

if(NOT HAVE_SYSTEM_JSON)
    FetchContent_Declare(
        json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG v3.11.2
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
    )
    FetchContent_MakeAvailable(json)
endif()

# 包含目录
include_directories(include)

# 添加vsomeip头文件路径
if(DEFINED VSOMEIP_INCLUDE_DIRS)
    include_directories(${VSOMEIP_INCLUDE_DIRS})
    message(STATUS "Added vsomeip include directories: ${VSOMEIP_INCLUDE_DIRS}")
endif()

# 平台特定的包含目录
if(UNIX AND NOT APPLE)
    # Linux特定头文件路径
    include_directories(/usr/include)
    include_directories(/usr/local/include)
endif()

# 源文件
set(COMMUNICATION_SOURCES
    src/communication/someip_client.cpp
    src/communication/door_service_client.cpp
    src/communication/window_service_client.cpp
    src/communication/light_service_client.cpp
    src/communication/seat_service_client.cpp
)

# Web API源文件
set(WEB_API_SOURCES
    src/web_api/http_server.cpp
    # websocket_server.cpp 已移除，使用SSE替代
    src/web_api/api_handlers.cpp
    src/web_api/json_converter.cpp
)

set(APPLICATION_SOURCES
    # 将来添加应用层源文件
)

set(INTERFACE_SOURCES
    # 将来添加接口层源文件
)

set(MANAGEMENT_SOURCES
    # 将来添加管理层源文件
)

# 创建主要的库
add_library(body_controller_lib STATIC
    ${COMMUNICATION_SOURCES}
    ${APPLICATION_SOURCES}
    ${INTERFACE_SOURCES}
    ${MANAGEMENT_SOURCES}
)

# 平台特定的链接库
set(PLATFORM_LIBS)
if(UNIX AND NOT APPLE)
    # Linux特定库
    list(APPEND PLATFORM_LIBS
        pthread
        dl
        rt
    )
    
    # 如果找到Boost，链接它
    if(Boost_FOUND)
        list(APPEND PLATFORM_LIBS ${Boost_LIBRARIES})
    endif()
elseif(WIN32)
    # Windows特定库
    list(APPEND PLATFORM_LIBS
        ws2_32
        wsock32
    )
endif()

# 链接库
target_link_libraries(body_controller_lib
    Threads::Threads
    ${VSOMEIP_LIBRARIES}
    ${PLATFORM_LIBS}
)

# 测试程序：车门服务客户端测试
add_executable(test_door_client
    src/test_door_client.cpp
)

target_link_libraries(test_door_client
    body_controller_lib
    ${VSOMEIP_LIBRARIES}
)

# 测试程序：车窗服务客户端测试
add_executable(test_window_client
    src/test_window_client.cpp
)

target_link_libraries(test_window_client
    body_controller_lib
    ${VSOMEIP_LIBRARIES}
)

# 测试程序：灯光服务客户端测试
add_executable(test_light_client
    src/test_light_client.cpp
)

target_link_libraries(test_light_client
    body_controller_lib
    ${VSOMEIP_LIBRARIES}
)

# 测试程序：座椅服务客户端测试
add_executable(test_seat_client
    src/test_seat_client.cpp
)

target_link_libraries(test_seat_client
    body_controller_lib
    ${VSOMEIP_LIBRARIES}
)

# Web服务器程序
add_executable(body_controller_web_server
    src/main_web_server.cpp
    ${WEB_API_SOURCES}
)

target_link_libraries(body_controller_web_server
    body_controller_lib
    httplib
    nlohmann_json
    ${VSOMEIP_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
)

# 设置可执行文件权限 (Linux)
if(UNIX)
    set_target_properties(test_door_client PROPERTIES
        INSTALL_RPATH_USE_LINK_PATH TRUE
    )
    set_target_properties(test_window_client PROPERTIES
        INSTALL_RPATH_USE_LINK_PATH TRUE
    )
    set_target_properties(test_light_client PROPERTIES
        INSTALL_RPATH_USE_LINK_PATH TRUE
    )
    set_target_properties(test_seat_client PROPERTIES
        INSTALL_RPATH_USE_LINK_PATH TRUE
    )
    set_target_properties(body_controller_web_server PROPERTIES
        INSTALL_RPATH_USE_LINK_PATH TRUE
    )
endif()

# Web界面资源复制
if(EXISTS ${CMAKE_SOURCE_DIR}/web/)
    file(COPY ${CMAKE_SOURCE_DIR}/web/ DESTINATION ${CMAKE_BINARY_DIR}/web/)
endif()

# 配置文件复制
if(EXISTS ${CMAKE_SOURCE_DIR}/config/)
    file(COPY ${CMAKE_SOURCE_DIR}/config/ DESTINATION ${CMAKE_BINARY_DIR}/config/)
endif()

# 测试
enable_testing()

# 添加简单的测试
add_test(NAME test_door_client_help
         COMMAND test_door_client --help
         WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

add_test(NAME test_window_client_help
         COMMAND test_window_client --help
         WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

add_test(NAME test_light_client_help
         COMMAND test_light_client --help
         WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

add_test(NAME test_seat_client_help
         COMMAND test_seat_client --help
         WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

# 安装规则
install(TARGETS test_door_client test_window_client test_light_client test_seat_client body_controller_web_server
    RUNTIME DESTINATION bin
    PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
)

# 安装Web文件
install(DIRECTORY web/
    DESTINATION web
    FILES_MATCHING
    PATTERN "*.html"
    PATTERN "*.css"
    PATTERN "*.js"
    PATTERN "*.png"
    PATTERN "*.jpg"
    PATTERN "*.ico"
)

install(DIRECTORY config/
    DESTINATION share/body_controller/config
    FILE_PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
)

# 创建启动脚本 (Linux)
if(UNIX AND NOT APPLE)
    # 创建车门测试脚本
    file(WRITE ${CMAKE_BINARY_DIR}/run_door_test.sh
        "#!/bin/bash\n"
        "export VSOMEIP_CONFIGURATION=${CMAKE_BINARY_DIR}/config/vsomeip.json\n"
        "export VSOMEIP_APPLICATION_NAME=door_test_client\n"
        "echo \"Starting Door Service Client Test...\"\n"
        "echo \"Configuration: $VSOMEIP_CONFIGURATION\"\n"
        "echo \"Application: $VSOMEIP_APPLICATION_NAME\"\n"
        "echo \"Press Ctrl+C to exit\"\n"
        "${CMAKE_BINARY_DIR}/bin/test_door_client\n"
    )

    # 创建车窗测试脚本
    file(WRITE ${CMAKE_BINARY_DIR}/run_window_test.sh
        "#!/bin/bash\n"
        "export VSOMEIP_CONFIGURATION=${CMAKE_BINARY_DIR}/config/vsomeip.json\n"
        "export VSOMEIP_APPLICATION_NAME=window_test_client\n"
        "echo \"Starting Window Service Client Test...\"\n"
        "echo \"Configuration: $VSOMEIP_CONFIGURATION\"\n"
        "echo \"Application: $VSOMEIP_APPLICATION_NAME\"\n"
        "echo \"Press Ctrl+C to exit\"\n"
        "${CMAKE_BINARY_DIR}/bin/test_window_client\n"
    )

    # 创建灯光测试脚本
    file(WRITE ${CMAKE_BINARY_DIR}/run_light_test.sh
        "#!/bin/bash\n"
        "export VSOMEIP_CONFIGURATION=${CMAKE_BINARY_DIR}/config/vsomeip.json\n"
        "export VSOMEIP_APPLICATION_NAME=light_test_client\n"
        "echo \"Starting Light Service Client Test...\"\n"
        "echo \"Configuration: $VSOMEIP_CONFIGURATION\"\n"
        "echo \"Application: $VSOMEIP_APPLICATION_NAME\"\n"
        "echo \"Press Ctrl+C to exit\"\n"
        "${CMAKE_BINARY_DIR}/bin/test_light_client\n"
    )

    # 创建座椅测试脚本
    file(WRITE ${CMAKE_BINARY_DIR}/run_seat_test.sh
        "#!/bin/bash\n"
        "export VSOMEIP_CONFIGURATION=${CMAKE_BINARY_DIR}/config/vsomeip.json\n"
        "export VSOMEIP_APPLICATION_NAME=seat_test_client\n"
        "echo \"Starting Seat Service Client Test...\"\n"
        "echo \"Configuration: $VSOMEIP_CONFIGURATION\"\n"
        "echo \"Application: $VSOMEIP_APPLICATION_NAME\"\n"
        "echo \"Press Ctrl+C to exit\"\n"
        "${CMAKE_BINARY_DIR}/bin/test_seat_client\n"
    )

    # 设置脚本权限
    file(COPY ${CMAKE_BINARY_DIR}/run_door_test.sh
         DESTINATION ${CMAKE_BINARY_DIR}/
         FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)

    file(COPY ${CMAKE_BINARY_DIR}/run_window_test.sh
         DESTINATION ${CMAKE_BINARY_DIR}/
         FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)

    file(COPY ${CMAKE_BINARY_DIR}/run_light_test.sh
         DESTINATION ${CMAKE_BINARY_DIR}/
         FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)

    file(COPY ${CMAKE_BINARY_DIR}/run_seat_test.sh
         DESTINATION ${CMAKE_BINARY_DIR}/
         FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)

    # 创建Web服务器启动脚本
    file(WRITE ${CMAKE_BINARY_DIR}/run_web_server.sh
        "#!/bin/bash\n"
        "export VSOMEIP_CONFIGURATION=${CMAKE_BINARY_DIR}/config/vsomeip.json\n"
        "export VSOMEIP_APPLICATION_NAME=web_server\n"
        "echo \"Starting Body Controller Web Server...\"\n"
        "echo \"Configuration: $VSOMEIP_CONFIGURATION\"\n"
        "echo \"Application: $VSOMEIP_APPLICATION_NAME\"\n"
        "echo \"HTTP Server: http://localhost:8080\"\n"
        "echo \"SSE Events: http://localhost:8080/api/events\"\n"
        "echo \"Press Ctrl+C to exit\"\n"
        "${CMAKE_BINARY_DIR}/bin/body_controller_web_server --http-port 8080\n"
    )

    file(COPY ${CMAKE_BINARY_DIR}/run_web_server.sh
         DESTINATION ${CMAKE_BINARY_DIR}/
         FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
endif()

# 打印配置信息
message(STATUS "=== Body Controller Build Configuration ===")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "Platform: ${CMAKE_SYSTEM_NAME}")
message(STATUS "vsomeip libraries: ${VSOMEIP_LIBRARIES}")
if(Boost_FOUND)
    message(STATUS "Boost found: ${Boost_VERSION}")
else()
    message(STATUS "Boost: not found")
endif()
message(STATUS "==========================================")

# 构建说明
message(STATUS "")
message(STATUS "Build Instructions:")
message(STATUS "  mkdir build && cd build")
message(STATUS "  cmake ..")
message(STATUS "  make -j$(nproc)")
message(STATUS "")
message(STATUS "Run Instructions:")
message(STATUS "  cd build")
message(STATUS "  ./run_door_test.sh  # (Linux)")
message(STATUS "  or")
message(STATUS "  export VSOMEIP_CONFIGURATION=./config/vsomeip.json")
message(STATUS "  export VSOMEIP_APPLICATION_NAME=door_test_client")
message(STATUS "  ./bin/test_door_client")
message(STATUS "")
