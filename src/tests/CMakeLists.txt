# src/tests/CMakeLists.txt - 测试模块
# 所有测试程序和测试相关配置

# 检查是否启用测试
if(NOT BUILD_TESTS)
    message(STATUS "Tests disabled, skipping...")
    return()
endif()

# 检查通信层是否可用
if(NOT BUILD_COMMUNICATION_LAYER)
    message(WARNING "Tests require communication layer, but it's disabled")
    return()
endif()

# 测试程序列表
set(TEST_PROGRAMS "")

# 根据启用的服务添加对应的测试程序
if(BUILD_DOOR_SERVICE)
    list(APPEND TEST_PROGRAMS test_door_client)
endif()

if(BUILD_WINDOW_SERVICE)
    list(APPEND TEST_PROGRAMS test_window_client)
endif()

if(BUILD_LIGHT_SERVICE)
    list(APPEND TEST_PROGRAMS test_light_client)
endif()

if(BUILD_SEAT_SERVICE)
    list(APPEND TEST_PROGRAMS test_seat_client)
endif()

# 创建测试程序
foreach(TEST_PROGRAM ${TEST_PROGRAMS})
    # 检查源文件是否存在
    set(TEST_SOURCE "${CMAKE_SOURCE_DIR}/src/${TEST_PROGRAM}.cpp")
    if(EXISTS ${TEST_SOURCE})
        # 创建可执行文件
        add_executable(${TEST_PROGRAM} ${TEST_SOURCE})
        
        # 设置目标属性
        set_target_properties(${TEST_PROGRAM} PROPERTIES
            OUTPUT_NAME "${TEST_PROGRAM}"
            VERSION ${PROJECT_VERSION}
        )
        
        # 链接库
        target_link_libraries(${TEST_PROGRAM}
            PRIVATE
                body_controller::communication
                body_controller::dependencies
                body_controller::compiler_settings
                body_controller::build_options
        )
        
        # 应用编译器设置
        apply_compiler_settings(${TEST_PROGRAM})
        apply_hardening_flags(${TEST_PROGRAM})
        apply_debug_options(${TEST_PROGRAM})
        configure_logging(${TEST_PROGRAM})
        
        # 设置运行时路径
        if(UNIX)
            set_target_properties(${TEST_PROGRAM} PROPERTIES
                INSTALL_RPATH_USE_LINK_PATH TRUE
                INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib"
            )
        endif()
        
        message(STATUS "Added test program: ${TEST_PROGRAM}")
    else()
        message(WARNING "Test source file not found: ${TEST_SOURCE}")
    endif()
endforeach()

# 创建测试脚本
function(create_test_script test_name service_name)
    # Linux测试脚本
    if(UNIX)
        set(SCRIPT_CONTENT "#!/bin/bash
# Auto-generated test script for ${test_name}

# 设置环境变量
export VSOMEIP_CONFIGURATION=\${VSOMEIP_CONFIGURATION:-\"./config/vsomeip.json\"}
export VSOMEIP_APPLICATION_NAME=\"${service_name}_test_client\"

# 检查配置文件
if [ ! -f \"\$VSOMEIP_CONFIGURATION\" ]; then
    echo \"Error: vsomeip configuration file not found: \$VSOMEIP_CONFIGURATION\"
    echo \"Please ensure the configuration file exists or set VSOMEIP_CONFIGURATION environment variable\"
    exit 1
fi

# 检查可执行文件
if [ ! -f \"./bin/${test_name}\" ]; then
    echo \"Error: Test executable not found: ./bin/${test_name}\"
    echo \"Please build the project first\"
    exit 1
fi

echo \"Starting ${test_name} with configuration: \$VSOMEIP_CONFIGURATION\"
echo \"Application name: \$VSOMEIP_APPLICATION_NAME\"
echo \"\"

# 运行测试程序
./bin/${test_name}
")
        
        file(WRITE ${CMAKE_BINARY_DIR}/run_${test_name}.sh "${SCRIPT_CONTENT}")
        
        # 设置执行权限
        file(COPY ${CMAKE_BINARY_DIR}/run_${test_name}.sh
            DESTINATION ${CMAKE_BINARY_DIR}
            FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                            GROUP_READ GROUP_EXECUTE
                            WORLD_READ WORLD_EXECUTE
        )
    endif()
    
    # Windows批处理脚本
    if(WIN32)
        set(SCRIPT_CONTENT "@echo off
REM Auto-generated test script for ${test_name}

REM 设置环境变量
if not defined VSOMEIP_CONFIGURATION set VSOMEIP_CONFIGURATION=.\\config\\vsomeip.json
set VSOMEIP_APPLICATION_NAME=${service_name}_test_client

REM 检查配置文件
if not exist \"%VSOMEIP_CONFIGURATION%\" (
    echo Error: vsomeip configuration file not found: %VSOMEIP_CONFIGURATION%
    echo Please ensure the configuration file exists or set VSOMEIP_CONFIGURATION environment variable
    exit /b 1
)

REM 检查可执行文件
if not exist \".\\bin\\${test_name}.exe\" (
    echo Error: Test executable not found: .\\bin\\${test_name}.exe
    echo Please build the project first
    exit /b 1
)

echo Starting ${test_name} with configuration: %VSOMEIP_CONFIGURATION%
echo Application name: %VSOMEIP_APPLICATION_NAME%
echo.

REM 运行测试程序
.\\bin\\${test_name}.exe
")
        
        file(WRITE ${CMAKE_BINARY_DIR}/run_${test_name}.bat "${SCRIPT_CONTENT}")
    endif()
endfunction()

# 为每个测试程序创建启动脚本
if(BUILD_DOOR_SERVICE AND TARGET test_door_client)
    create_test_script("test_door_client" "door")
endif()

if(BUILD_WINDOW_SERVICE AND TARGET test_window_client)
    create_test_script("test_window_client" "window")
endif()

if(BUILD_LIGHT_SERVICE AND TARGET test_light_client)
    create_test_script("test_light_client" "light")
endif()

if(BUILD_SEAT_SERVICE AND TARGET test_seat_client)
    create_test_script("test_seat_client" "seat")
endif()

# 创建总体测试脚本
if(UNIX)
    set(ALL_TESTS_SCRIPT "#!/bin/bash
# Auto-generated script to run all tests

echo \"Running all Body Controller tests...\"
echo \"\"

FAILED_TESTS=()
")
    
    foreach(TEST_PROGRAM ${TEST_PROGRAMS})
        if(TARGET ${TEST_PROGRAM})
            set(ALL_TESTS_SCRIPT "${ALL_TESTS_SCRIPT}
echo \"Running ${TEST_PROGRAM}...\"
if ./run_${TEST_PROGRAM}.sh; then
    echo \"${TEST_PROGRAM}: PASSED\"
else
    echo \"${TEST_PROGRAM}: FAILED\"
    FAILED_TESTS+=(\"${TEST_PROGRAM}\")
fi
echo \"\"
")
        endif()
    endforeach()
    
    set(ALL_TESTS_SCRIPT "${ALL_TESTS_SCRIPT}
if [ \${#FAILED_TESTS[@]} -eq 0 ]; then
    echo \"All tests PASSED!\"
    exit 0
else
    echo \"Failed tests: \${FAILED_TESTS[*]}\"
    exit 1
fi
")
    
    file(WRITE ${CMAKE_BINARY_DIR}/run_all_tests.sh "${ALL_TESTS_SCRIPT}")
    file(COPY ${CMAKE_BINARY_DIR}/run_all_tests.sh
        DESTINATION ${CMAKE_BINARY_DIR}
        FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                        GROUP_READ GROUP_EXECUTE
                        WORLD_READ WORLD_EXECUTE
    )
endif()

# 安装测试程序
if(INSTALL_DEVELOPMENT_FILES)
    foreach(TEST_PROGRAM ${TEST_PROGRAMS})
        if(TARGET ${TEST_PROGRAM})
            install(TARGETS ${TEST_PROGRAM}
                RUNTIME DESTINATION bin
            )
        endif()
    endforeach()
    
    # 安装测试脚本
    if(UNIX)
        foreach(TEST_PROGRAM ${TEST_PROGRAMS})
            if(TARGET ${TEST_PROGRAM})
                install(FILES ${CMAKE_BINARY_DIR}/run_${TEST_PROGRAM}.sh
                    DESTINATION bin
                    PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                                GROUP_READ GROUP_EXECUTE
                                WORLD_READ WORLD_EXECUTE
                )
            endif()
        endforeach()
        
        install(FILES ${CMAKE_BINARY_DIR}/run_all_tests.sh
            DESTINATION bin
            PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                        GROUP_READ GROUP_EXECUTE
                        WORLD_READ WORLD_EXECUTE
        )
    endif()
    
    if(WIN32)
        foreach(TEST_PROGRAM ${TEST_PROGRAMS})
            if(TARGET ${TEST_PROGRAM})
                install(FILES ${CMAKE_BINARY_DIR}/run_${TEST_PROGRAM}.bat
                    DESTINATION bin
                )
            endif()
        endforeach()
    endif()
endif()

# 添加CTest支持
if(BUILD_TESTS AND CMAKE_VERSION VERSION_GREATER_EQUAL "3.10")
    enable_testing()
    
    foreach(TEST_PROGRAM ${TEST_PROGRAMS})
        if(TARGET ${TEST_PROGRAM})
            add_test(NAME ${TEST_PROGRAM}
                COMMAND ${TEST_PROGRAM}
                WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            )
            
            # 设置测试环境变量
            set_tests_properties(${TEST_PROGRAM} PROPERTIES
                ENVIRONMENT "VSOMEIP_CONFIGURATION=${CMAKE_BINARY_DIR}/config/vsomeip.json;VSOMEIP_APPLICATION_NAME=${TEST_PROGRAM}"
                TIMEOUT 30
            )
        endif()
    endforeach()
    
    # 创建自定义测试目标
    add_custom_target(run_tests
        COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
        DEPENDS ${TEST_PROGRAMS}
        COMMENT "Running all tests"
    )
endif()

# 打印测试模块信息
message(STATUS "")
message(STATUS "=== Test Module ===")
message(STATUS "Test Programs: ${TEST_PROGRAMS}")
message(STATUS "CTest Support: ${BUILD_TESTS}")
message(STATUS "===================")
message(STATUS "")
