# web/CMakeLists.txt - 前端静态文件模块
# 处理和安装Web前端资源

# 检查是否启用Web前端
if(NOT BUILD_WEB_FRONTEND)
    message(STATUS "Web frontend disabled, skipping...")
    return()
endif()

# 检查Web API是否启用
if(NOT BUILD_WEB_API)
    message(STATUS "Web API disabled, skipping web frontend...")
    return()
endif()

# Web文件目录结构
set(WEB_DIRECTORIES
    css
    js
    images
    fonts
)

# 主要Web文件
set(WEB_FILES
    index.html
)

# CSS文件
set(CSS_FILES "")
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/css)
    file(GLOB CSS_FILES ${CMAKE_CURRENT_SOURCE_DIR}/css/*.css)
endif()

# JavaScript文件
set(JS_FILES "")
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/js)
    file(GLOB JS_FILES ${CMAKE_CURRENT_SOURCE_DIR}/js/*.js)
endif()

# 图片文件
set(IMAGE_FILES "")
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/images)
    file(GLOB IMAGE_FILES 
        ${CMAKE_CURRENT_SOURCE_DIR}/images/*.png
        ${CMAKE_CURRENT_SOURCE_DIR}/images/*.jpg
        ${CMAKE_CURRENT_SOURCE_DIR}/images/*.jpeg
        ${CMAKE_CURRENT_SOURCE_DIR}/images/*.gif
        ${CMAKE_CURRENT_SOURCE_DIR}/images/*.svg
        ${CMAKE_CURRENT_SOURCE_DIR}/images/*.ico
    )
endif()

# 字体文件
set(FONT_FILES "")
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/fonts)
    file(GLOB FONT_FILES 
        ${CMAKE_CURRENT_SOURCE_DIR}/fonts/*.woff
        ${CMAKE_CURRENT_SOURCE_DIR}/fonts/*.woff2
        ${CMAKE_CURRENT_SOURCE_DIR}/fonts/*.ttf
        ${CMAKE_CURRENT_SOURCE_DIR}/fonts/*.otf
        ${CMAKE_CURRENT_SOURCE_DIR}/fonts/*.eot
    )
endif()

# 检查主要文件是否存在
foreach(WEB_FILE ${WEB_FILES})
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${WEB_FILE})
        message(STATUS "Found web file: ${WEB_FILE}")
    else()
        message(WARNING "Web file not found: ${WEB_FILE}")
    endif()
endforeach()

# 处理HTML模板文件
function(process_html_template input_file output_file)
    # 设置模板变量
    set(PROJECT_NAME ${PROJECT_NAME})
    set(PROJECT_VERSION ${PROJECT_VERSION})
    set(API_BASE_URL "http://localhost:8080/api")
    set(SSE_URL "http://localhost:8080/api/events")  # 使用SSE替代WebSocket
    
    # 功能开关
    set(ENABLE_DOOR_CONTROL ${BUILD_DOOR_SERVICE})
    set(ENABLE_WINDOW_CONTROL ${BUILD_WINDOW_SERVICE})
    set(ENABLE_LIGHT_CONTROL ${BUILD_LIGHT_SERVICE})
    set(ENABLE_SEAT_CONTROL ${BUILD_SEAT_SERVICE})
    
    # 处理模板
    configure_file(${input_file} ${output_file} @ONLY)
endfunction()

# 处理主HTML文件
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/index.html)
    # 检查是否是模板文件
    file(READ ${CMAKE_CURRENT_SOURCE_DIR}/index.html HTML_CONTENT)
    string(FIND "${HTML_CONTENT}" "@" TEMPLATE_FOUND)
    
    if(TEMPLATE_FOUND GREATER -1)
        # 处理模板文件
        process_html_template(
            ${CMAKE_CURRENT_SOURCE_DIR}/index.html
            ${CMAKE_BINARY_DIR}/web/index.html
        )
    else()
        # 直接复制文件
        configure_file(
            ${CMAKE_CURRENT_SOURCE_DIR}/index.html
            ${CMAKE_BINARY_DIR}/web/index.html
            COPYONLY
        )
    endif()
endif()

# 复制CSS文件
if(CSS_FILES)
    file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/web/css)
    foreach(CSS_FILE ${CSS_FILES})
        get_filename_component(CSS_NAME ${CSS_FILE} NAME)
        configure_file(${CSS_FILE} ${CMAKE_BINARY_DIR}/web/css/${CSS_NAME} COPYONLY)
    endforeach()
    message(STATUS "Copied ${list(LENGTH CSS_FILES)} CSS files")
endif()

# 复制JavaScript文件
if(JS_FILES)
    file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/web/js)
    foreach(JS_FILE ${JS_FILES})
        get_filename_component(JS_NAME ${JS_FILE} NAME)
        
        # 检查是否是模板文件
        file(READ ${JS_FILE} JS_CONTENT)
        string(FIND "${JS_CONTENT}" "@" JS_TEMPLATE_FOUND)
        
        if(JS_TEMPLATE_FOUND GREATER -1)
            # 处理JavaScript模板
            set(API_BASE_URL "http://localhost:8080/api")
            set(SSE_URL "http://localhost:8080/api/events")  # 使用SSE替代WebSocket
            configure_file(${JS_FILE} ${CMAKE_BINARY_DIR}/web/js/${JS_NAME} @ONLY)
        else()
            # 直接复制
            configure_file(${JS_FILE} ${CMAKE_BINARY_DIR}/web/js/${JS_NAME} COPYONLY)
        endif()
    endforeach()
    message(STATUS "Copied ${list(LENGTH JS_FILES)} JavaScript files")
endif()

# 复制图片文件
if(IMAGE_FILES)
    file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/web/images)
    foreach(IMAGE_FILE ${IMAGE_FILES})
        get_filename_component(IMAGE_NAME ${IMAGE_FILE} NAME)
        configure_file(${IMAGE_FILE} ${CMAKE_BINARY_DIR}/web/images/${IMAGE_NAME} COPYONLY)
    endforeach()
    message(STATUS "Copied ${list(LENGTH IMAGE_FILES)} image files")
endif()

# 复制字体文件
if(FONT_FILES)
    file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/web/fonts)
    foreach(FONT_FILE ${FONT_FILES})
        get_filename_component(FONT_NAME ${FONT_FILE} NAME)
        configure_file(${FONT_FILE} ${CMAKE_BINARY_DIR}/web/fonts/${FONT_NAME} COPYONLY)
    endforeach()
    message(STATUS "Copied ${list(LENGTH FONT_FILES)} font files")
endif()

# 创建Web资源清单文件
set(MANIFEST_CONTENT "{
    \"name\": \"Body Controller Web Interface\",
    \"version\": \"${PROJECT_VERSION}\",
    \"description\": \"Web interface for Body Controller System\",
    \"files\": {
        \"html\": [")

foreach(WEB_FILE ${WEB_FILES})
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${WEB_FILE})
        set(MANIFEST_CONTENT "${MANIFEST_CONTENT}
            \"${WEB_FILE}\"")
    endif()
endforeach()

set(MANIFEST_CONTENT "${MANIFEST_CONTENT}
        ],
        \"css\": [")

foreach(CSS_FILE ${CSS_FILES})
    get_filename_component(CSS_NAME ${CSS_FILE} NAME)
    set(MANIFEST_CONTENT "${MANIFEST_CONTENT}
            \"css/${CSS_NAME}\"")
endforeach()

set(MANIFEST_CONTENT "${MANIFEST_CONTENT}
        ],
        \"js\": [")

foreach(JS_FILE ${JS_FILES})
    get_filename_component(JS_NAME ${JS_FILE} NAME)
    set(MANIFEST_CONTENT "${MANIFEST_CONTENT}
            \"js/${JS_NAME}\"")
endforeach()

set(MANIFEST_CONTENT "${MANIFEST_CONTENT}
        ]
    },
    \"features\": {
        \"door_control\": ${BUILD_DOOR_SERVICE},
        \"window_control\": ${BUILD_WINDOW_SERVICE},
        \"light_control\": ${BUILD_LIGHT_SERVICE},
        \"seat_control\": ${BUILD_SEAT_SERVICE}
    },
    \"api\": {
        \"base_url\": \"http://localhost:8080/api\",
        \"sse_url\": \"http://localhost:8080/api/events\"
    }
}")

file(WRITE ${CMAKE_BINARY_DIR}/web/manifest.json "${MANIFEST_CONTENT}")

# 创建开发服务器脚本（用于前端开发）
if(UNIX)
    set(DEV_SERVER_SCRIPT "#!/bin/bash
# Development web server script

WEB_DIR=\"${CMAKE_BINARY_DIR}/web\"
PORT=\${1:-8000}

if [ ! -d \"\$WEB_DIR\" ]; then
    echo \"Error: Web directory not found: \$WEB_DIR\"
    echo \"Please build the project first\"
    exit 1
fi

echo \"Starting development web server...\"
echo \"Web directory: \$WEB_DIR\"
echo \"Port: \$PORT\"
echo \"URL: http://localhost:\$PORT\"
echo \"\"
echo \"Note: This is for development only. Use the main web server for production.\"
echo \"\"

cd \"\$WEB_DIR\"

# 尝试使用Python 3的http.server
if command -v python3 &> /dev/null; then
    python3 -m http.server \$PORT
elif command -v python &> /dev/null; then
    python -m SimpleHTTPServer \$PORT
else
    echo \"Error: Python not found. Please install Python to use the development server.\"
    exit 1
fi
")
    
    file(WRITE ${CMAKE_BINARY_DIR}/start_dev_server.sh "${DEV_SERVER_SCRIPT}")
    file(COPY ${CMAKE_BINARY_DIR}/start_dev_server.sh
        DESTINATION ${CMAKE_BINARY_DIR}
        FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                        GROUP_READ GROUP_EXECUTE
                        WORLD_READ WORLD_EXECUTE
    )
endif()

# 安装Web文件
install(DIRECTORY ${CMAKE_BINARY_DIR}/web/
    DESTINATION web
    FILES_MATCHING 
        PATTERN "*.html"
        PATTERN "*.css"
        PATTERN "*.js"
        PATTERN "*.png"
        PATTERN "*.jpg"
        PATTERN "*.jpeg"
        PATTERN "*.gif"
        PATTERN "*.svg"
        PATTERN "*.ico"
        PATTERN "*.woff"
        PATTERN "*.woff2"
        PATTERN "*.ttf"
        PATTERN "*.otf"
        PATTERN "*.eot"
        PATTERN "*.json"
)

# 创建自定义目标用于Web资源处理
add_custom_target(web_resources ALL
    DEPENDS ${CMAKE_BINARY_DIR}/web/index.html
    COMMENT "Processing web resources"
)

# 添加Web资源的依赖关系
if(TARGET body_controller_web_server)
    add_dependencies(body_controller_web_server web_resources)
endif()

# 创建Web资源清理目标
add_custom_target(clean_web
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/web
    COMMENT "Cleaning web resources"
)

# 打印Web模块信息
message(STATUS "")
message(STATUS "=== Web Frontend Module ===")
message(STATUS "HTML files: ${WEB_FILES}")
message(STATUS "CSS files: ${list(LENGTH CSS_FILES)} files")
message(STATUS "JS files: ${list(LENGTH JS_FILES)} files")
message(STATUS "Image files: ${list(LENGTH IMAGE_FILES)} files")
message(STATUS "Font files: ${list(LENGTH FONT_FILES)} files")
message(STATUS "Output directory: ${CMAKE_BINARY_DIR}/web")
message(STATUS "============================")
message(STATUS "")
