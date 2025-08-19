# config/CMakeLists.txt - 配置文件模块
# 处理和安装配置文件

# 配置文件列表
set(CONFIG_FILES
    vsomeip.json
    system_config.json
)

# 检查配置文件是否存在
foreach(CONFIG_FILE ${CONFIG_FILES})
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${CONFIG_FILE})
        message(STATUS "Found config file: ${CONFIG_FILE}")
    else()
        message(WARNING "Config file not found: ${CONFIG_FILE}")
    endif()
endforeach()

# 创建配置文件模板处理函数
function(configure_template_file input_file output_file)
    # 设置模板变量
    set(PROJECT_NAME ${PROJECT_NAME})
    set(PROJECT_VERSION ${PROJECT_VERSION})
    set(CMAKE_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})
    set(CMAKE_BINARY_DIR ${CMAKE_BINARY_DIR})
    
    # Web服务器配置
    set(DEFAULT_HTTP_PORT 8080)
    set(DEFAULT_WEBSOCKET_PORT 8081)
    set(DEFAULT_WEB_ROOT "./web")
    
    # SOME/IP配置
    set(DEFAULT_UNICAST_ADDRESS "127.0.0.1")
    set(DEFAULT_NETMASK "255.255.255.0")
    set(DEFAULT_MULTICAST_ADDRESS "224.0.0.1")
    set(DEFAULT_MULTICAST_PORT 30490)
    
    # 服务配置
    set(DOOR_SERVICE_ID "0x1002")
    set(WINDOW_SERVICE_ID "0x1001")
    set(LIGHT_SERVICE_ID "0x1003")
    set(SEAT_SERVICE_ID "0x1004")
    
    set(DOOR_INSTANCE_ID "0x1002")
    set(WINDOW_INSTANCE_ID "0x1001")
    set(LIGHT_INSTANCE_ID "0x1003")
    set(SEAT_INSTANCE_ID "0x1004")
    
    # 处理模板文件
    configure_file(${input_file} ${output_file} @ONLY)
endfunction()

# 处理vsomeip配置文件
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/vsomeip.json)
    # 直接复制现有配置文件
    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/vsomeip.json
        ${CMAKE_BINARY_DIR}/config/vsomeip.json
        COPYONLY
    )
else()
    # 如果存在模板文件，则处理模板
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/vsomeip.json.in)
        configure_template_file(
            ${CMAKE_CURRENT_SOURCE_DIR}/vsomeip.json.in
            ${CMAKE_BINARY_DIR}/config/vsomeip.json
        )
    else()
        # 创建默认的vsomeip配置文件
        set(VSOMEIP_CONFIG_CONTENT "{
    \"unicast\" : \"@DEFAULT_UNICAST_ADDRESS@\",
    \"netmask\" : \"@DEFAULT_NETMASK@\",
    \"logging\" :
    {
        \"level\" : \"info\",
        \"console\" : \"true\",
        \"file\" : { \"enable\" : \"false\", \"path\" : \"/tmp\" },
        \"dlt\" : \"false\"
    },
    \"applications\" :
    [
        {
            \"name\" : \"door_test_client\",
            \"id\" : \"0x1111\"
        },
        {
            \"name\" : \"window_test_client\",
            \"id\" : \"0x2222\"
        },
        {
            \"name\" : \"light_test_client\",
            \"id\" : \"0x3333\"
        },
        {
            \"name\" : \"seat_test_client\",
            \"id\" : \"0x4444\"
        },
        {
            \"name\" : \"web_server\",
            \"id\" : \"0x5555\"
        }
    ],
    \"services\" :
    [
        {
            \"service\" : \"@DOOR_SERVICE_ID@\",
            \"instance\" : \"@DOOR_INSTANCE_ID@\",
            \"unreliable\" : \"@DEFAULT_MULTICAST_PORT@\"
        },
        {
            \"service\" : \"@WINDOW_SERVICE_ID@\",
            \"instance\" : \"@WINDOW_INSTANCE_ID@\",
            \"unreliable\" : \"@DEFAULT_MULTICAST_PORT@\"
        },
        {
            \"service\" : \"@LIGHT_SERVICE_ID@\",
            \"instance\" : \"@LIGHT_INSTANCE_ID@\",
            \"unreliable\" : \"@DEFAULT_MULTICAST_PORT@\"
        },
        {
            \"service\" : \"@SEAT_SERVICE_ID@\",
            \"instance\" : \"@SEAT_INSTANCE_ID@\",
            \"unreliable\" : \"@DEFAULT_MULTICAST_PORT@\"
        }
    ],
    \"routing\" : \"door_test_client\",
    \"service-discovery\" :
    {
        \"enable\" : \"true\",
        \"multicast\" : \"@DEFAULT_MULTICAST_ADDRESS@\",
        \"port\" : \"@DEFAULT_MULTICAST_PORT@\",
        \"protocol\" : \"udp\",
        \"initial_delay_min\" : \"10\",
        \"initial_delay_max\" : \"100\",
        \"repetitions_base_delay\" : \"200\",
        \"repetitions_max\" : \"3\",
        \"ttl\" : \"3\",
        \"cyclic_offer_delay\" : \"2000\",
        \"request_response_delay\" : \"1500\"
    }
}")
        
        file(WRITE ${CMAKE_CURRENT_SOURCE_DIR}/vsomeip.json.in "${VSOMEIP_CONFIG_CONTENT}")
        configure_template_file(
            ${CMAKE_CURRENT_SOURCE_DIR}/vsomeip.json.in
            ${CMAKE_BINARY_DIR}/config/vsomeip.json
        )
    endif()
endif()

# 处理系统配置文件
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/system_config.json)
    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/system_config.json
        ${CMAKE_BINARY_DIR}/config/system_config.json
        COPYONLY
    )
else()
    # 创建默认系统配置文件
    if(BUILD_WEB_API)
        set(SYSTEM_CONFIG_CONTENT "{
    \"web_server\": {
        \"http_port\": @DEFAULT_HTTP_PORT@,
        \"websocket_port\": @DEFAULT_WEBSOCKET_PORT@,
        \"web_root\": \"@DEFAULT_WEB_ROOT@\",
        \"enable_cors\": true,
        \"max_connections\": 100,
        \"request_timeout_ms\": 5000,
        \"response_timeout_ms\": 10000
    },
    \"communication\": {
        \"vsomeip_config\": \"./config/vsomeip.json\",
        \"application_name\": \"web_server\",
        \"service_discovery_timeout_ms\": 5000
    },
    \"logging\": {
        \"level\": \"info\",
        \"enable_console\": true,
        \"enable_file\": false,
        \"log_file\": \"./logs/body_controller.log\"
    },
    \"services\": {
        \"door_service\": {
            \"enabled\": @BUILD_DOOR_SERVICE@,
            \"service_id\": \"@DOOR_SERVICE_ID@\",
            \"instance_id\": \"@DOOR_INSTANCE_ID@\"
        },
        \"window_service\": {
            \"enabled\": @BUILD_WINDOW_SERVICE@,
            \"service_id\": \"@WINDOW_SERVICE_ID@\",
            \"instance_id\": \"@WINDOW_INSTANCE_ID@\"
        },
        \"light_service\": {
            \"enabled\": @BUILD_LIGHT_SERVICE@,
            \"service_id\": \"@LIGHT_SERVICE_ID@\",
            \"instance_id\": \"@LIGHT_INSTANCE_ID@\"
        },
        \"seat_service\": {
            \"enabled\": @BUILD_SEAT_SERVICE@,
            \"service_id\": \"@SEAT_SERVICE_ID@\",
            \"instance_id\": \"@SEAT_INSTANCE_ID@\"
        }
    }
}")
    else()
        set(SYSTEM_CONFIG_CONTENT "{
    \"communication\": {
        \"vsomeip_config\": \"./config/vsomeip.json\",
        \"service_discovery_timeout_ms\": 5000
    },
    \"logging\": {
        \"level\": \"info\",
        \"enable_console\": true,
        \"enable_file\": false,
        \"log_file\": \"./logs/body_controller.log\"
    },
    \"services\": {
        \"door_service\": {
            \"enabled\": @BUILD_DOOR_SERVICE@,
            \"service_id\": \"@DOOR_SERVICE_ID@\",
            \"instance_id\": \"@DOOR_INSTANCE_ID@\"
        },
        \"window_service\": {
            \"enabled\": @BUILD_WINDOW_SERVICE@,
            \"service_id\": \"@WINDOW_SERVICE_ID@\",
            \"instance_id\": \"@WINDOW_INSTANCE_ID@\"
        },
        \"light_service\": {
            \"enabled\": @BUILD_LIGHT_SERVICE@,
            \"service_id\": \"@LIGHT_SERVICE_ID@\",
            \"instance_id\": \"@LIGHT_INSTANCE_ID@\"
        },
        \"seat_service\": {
            \"enabled\": @BUILD_SEAT_SERVICE@,
            \"service_id\": \"@SEAT_SERVICE_ID@\",
            \"instance_id\": \"@SEAT_INSTANCE_ID@\"
        }
    }
}")
    endif()
    
    file(WRITE ${CMAKE_CURRENT_SOURCE_DIR}/system_config.json.in "${SYSTEM_CONFIG_CONTENT}")
    configure_template_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/system_config.json.in
        ${CMAKE_BINARY_DIR}/config/system_config.json
    )
endif()

# 创建Web服务器配置文件（如果启用Web API）
if(BUILD_WEB_API)
    set(WEB_SERVER_CONFIG_CONTENT "{
    \"server\": {
        \"http_port\": @DEFAULT_HTTP_PORT@,
        \"websocket_port\": @DEFAULT_WEBSOCKET_PORT@,
        \"host\": \"0.0.0.0\",
        \"web_root\": \"@DEFAULT_WEB_ROOT@\",
        \"enable_cors\": true,
        \"max_request_size\": 1048576,
        \"max_connections\": 100
    },
    \"security\": {
        \"enable_rate_limiting\": true,
        \"rate_limit_requests\": 100,
        \"rate_limit_window_ms\": 60000,
        \"enable_https\": false,
        \"ssl_cert_file\": \"\",
        \"ssl_key_file\": \"\"
    },
    \"timeouts\": {
        \"request_timeout_ms\": 5000,
        \"response_timeout_ms\": 10000,
        \"websocket_ping_interval_ms\": 30000
    },
    \"logging\": {
        \"level\": \"info\",
        \"enable_access_log\": true,
        \"access_log_file\": \"./logs/access.log\"
    }
}")
    
    file(WRITE ${CMAKE_CURRENT_SOURCE_DIR}/web_server_config.json.in "${WEB_SERVER_CONFIG_CONTENT}")
    configure_template_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/web_server_config.json.in
        ${CMAKE_BINARY_DIR}/config/web_server_config.json
    )
endif()

# 安装配置文件
install(DIRECTORY ${CMAKE_BINARY_DIR}/config/
    DESTINATION config
    FILES_MATCHING PATTERN "*.json"
)

# 创建配置验证脚本
if(UNIX)
    set(VALIDATE_CONFIG_SCRIPT "#!/bin/bash
# Configuration validation script

CONFIG_DIR=\"./config\"
ERRORS=0

echo \"Validating Body Controller configuration files...\"
echo \"\"

# 检查vsomeip配置
if [ -f \"\$CONFIG_DIR/vsomeip.json\" ]; then
    echo \"✓ vsomeip.json found\"
    # 可以添加JSON语法验证
else
    echo \"✗ vsomeip.json not found\"
    ERRORS=\$((ERRORS + 1))
fi

# 检查系统配置
if [ -f \"\$CONFIG_DIR/system_config.json\" ]; then
    echo \"✓ system_config.json found\"
else
    echo \"✗ system_config.json not found\"
    ERRORS=\$((ERRORS + 1))
fi

# 检查Web服务器配置（如果启用）
if [ -f \"\$CONFIG_DIR/web_server_config.json\" ]; then
    echo \"✓ web_server_config.json found\"
fi

echo \"\"
if [ \$ERRORS -eq 0 ]; then
    echo \"All configuration files are valid!\"
    exit 0
else
    echo \"Found \$ERRORS configuration errors\"
    exit 1
fi
")
    
    file(WRITE ${CMAKE_BINARY_DIR}/validate_config.sh "${VALIDATE_CONFIG_SCRIPT}")
    file(COPY ${CMAKE_BINARY_DIR}/validate_config.sh
        DESTINATION ${CMAKE_BINARY_DIR}
        FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                        GROUP_READ GROUP_EXECUTE
                        WORLD_READ WORLD_EXECUTE
    )
    
    install(FILES ${CMAKE_BINARY_DIR}/validate_config.sh
        DESTINATION bin
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                    GROUP_READ GROUP_EXECUTE
                    WORLD_READ WORLD_EXECUTE
    )
endif()

# 打印配置模块信息
message(STATUS "")
message(STATUS "=== Configuration Module ===")
message(STATUS "Config files: ${CONFIG_FILES}")
message(STATUS "Output directory: ${CMAKE_BINARY_DIR}/config")
message(STATUS "=============================")
message(STATUS "")
