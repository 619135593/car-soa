@echo off
setlocal enabledelayedexpansion

echo ========================================
echo 车身域控制器 Windows 编译脚本
echo ========================================
echo.

:: 设置颜色
set "RED=[91m"
set "GREEN=[92m"
set "YELLOW=[93m"
set "BLUE=[94m"
set "NC=[0m"

:: 获取脚本所在目录
set "SCRIPT_DIR=%~dp0"
set "PROJECT_ROOT=%SCRIPT_DIR%.."

:: 切换到项目根目录
cd /d "%PROJECT_ROOT%"

echo %BLUE%[INFO]%NC% 项目根目录: %PROJECT_ROOT%
echo.

:: 检查必要工具
echo %BLUE%[INFO]%NC% 检查开发工具...

:: 检查CMake
cmake --version >nul 2>&1
if errorlevel 1 (
    echo %RED%[ERROR]%NC% CMake 未安装或不在PATH中
    echo 请安装CMake: https://cmake.org/download/
    pause
    exit /b 1
)
echo %GREEN%[OK]%NC% CMake 已安装

:: 检查编译器
where cl >nul 2>&1
if errorlevel 1 (
    echo %YELLOW%[WARNING]%NC% Visual Studio编译器未找到，尝试设置环境...
    
    :: 尝试找到Visual Studio
    set "VS_PATHS="
    set "VS_PATHS=%VS_PATHS% C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
    set "VS_PATHS=%VS_PATHS% C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvars64.bat"
    set "VS_PATHS=%VS_PATHS% C:\Program Files\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"
    set "VS_PATHS=%VS_PATHS% C:\Program Files\Microsoft Visual Studio\2019\Professional\VC\Auxiliary\Build\vcvars64.bat"
    
    set "VS_FOUND=0"
    for %%p in (%VS_PATHS%) do (
        if exist "%%p" (
            echo %BLUE%[INFO]%NC% 找到Visual Studio: %%p
            call "%%p"
            set "VS_FOUND=1"
            goto :vs_found
        )
    )
    
    :vs_found
    if !VS_FOUND! == 0 (
        echo %RED%[ERROR]%NC% 未找到Visual Studio编译器
        echo 请安装Visual Studio 2019/2022 Community版本
        echo 下载地址: https://visualstudio.microsoft.com/zh-hans/vs/community/
        pause
        exit /b 1
    )
) else (
    echo %GREEN%[OK]%NC% Visual Studio编译器已就绪
)

:: 检查Git
git --version >nul 2>&1
if errorlevel 1 (
    echo %YELLOW%[WARNING]%NC% Git 未安装，某些功能可能不可用
) else (
    echo %GREEN%[OK]%NC% Git 已安装
)

echo.

:: 创建并进入build目录
if not exist "build" (
    echo %BLUE%[INFO]%NC% 创建build目录...
    mkdir build
)

cd build

:: 清理之前的构建（可选）
set /p "CLEAN=是否清理之前的构建? (y/N): "
if /i "%CLEAN%"=="y" (
    echo %BLUE%[INFO]%NC% 清理build目录...
    del /q /s * >nul 2>&1
    for /d %%d in (*) do rmdir /s /q "%%d" >nul 2>&1
)

:: 配置CMake
echo %BLUE%[INFO]%NC% 配置CMake...
echo.

:: 尝试不同的生成器
set "CMAKE_SUCCESS=0"

:: 首先尝试Visual Studio 2022
echo %BLUE%[INFO]%NC% 尝试使用 Visual Studio 2022...
cmake -G "Visual Studio 17 2022" -A x64 .. >cmake_config.log 2>&1
if !errorlevel! == 0 (
    set "CMAKE_SUCCESS=1"
    set "GENERATOR=Visual Studio 17 2022"
    echo %GREEN%[SUCCESS]%NC% CMake配置成功 (Visual Studio 2022)
    goto :cmake_success
)

:: 尝试Visual Studio 2019
echo %BLUE%[INFO]%NC% 尝试使用 Visual Studio 2019...
cmake -G "Visual Studio 16 2019" -A x64 .. >cmake_config.log 2>&1
if !errorlevel! == 0 (
    set "CMAKE_SUCCESS=1"
    set "GENERATOR=Visual Studio 16 2019"
    echo %GREEN%[SUCCESS]%NC% CMake配置成功 (Visual Studio 2019)
    goto :cmake_success
)

:: 尝试MinGW
echo %BLUE%[INFO]%NC% 尝试使用 MinGW Makefiles...
cmake -G "MinGW Makefiles" .. >cmake_config.log 2>&1
if !errorlevel! == 0 (
    set "CMAKE_SUCCESS=1"
    set "GENERATOR=MinGW Makefiles"
    echo %GREEN%[SUCCESS]%NC% CMake配置成功 (MinGW)
    goto :cmake_success
)

:: 尝试Ninja（如果可用）
ninja --version >nul 2>&1
if !errorlevel! == 0 (
    echo %BLUE%[INFO]%NC% 尝试使用 Ninja...
    cmake -G "Ninja" .. >cmake_config.log 2>&1
    if !errorlevel! == 0 (
        set "CMAKE_SUCCESS=1"
        set "GENERATOR=Ninja"
        echo %GREEN%[SUCCESS]%NC% CMake配置成功 (Ninja)
        goto :cmake_success
    )
)

:cmake_success
if !CMAKE_SUCCESS! == 0 (
    echo %RED%[ERROR]%NC% CMake配置失败
    echo 错误日志:
    type cmake_config.log
    echo.
    echo 请检查:
    echo 1. Visual Studio 是否正确安装
    echo 2. CMake 版本是否兼容
    echo 3. 项目依赖是否满足
    pause
    exit /b 1
)

echo.

:: 编译项目
echo %BLUE%[INFO]%NC% 开始编译项目...
echo 使用生成器: !GENERATOR!
echo.

if "!GENERATOR!"=="MinGW Makefiles" (
    :: 使用MinGW编译
    mingw32-make -j%NUMBER_OF_PROCESSORS%
    set "BUILD_RESULT=!errorlevel!"
) else if "!GENERATOR!"=="Ninja" (
    :: 使用Ninja编译
    ninja
    set "BUILD_RESULT=!errorlevel!"
) else (
    :: 使用Visual Studio编译
    cmake --build . --config Release --parallel %NUMBER_OF_PROCESSORS%
    set "BUILD_RESULT=!errorlevel!"
)

if !BUILD_RESULT! neq 0 (
    echo %RED%[ERROR]%NC% 编译失败
    echo.
    echo 常见解决方案:
    echo 1. 检查源代码是否有语法错误
    echo 2. 确认所有依赖库已安装
    echo 3. 尝试清理后重新编译
    echo 4. 检查编译器版本兼容性
    pause
    exit /b 1
)

echo.
echo %GREEN%[SUCCESS]%NC% 编译完成！

:: 检查生成的可执行文件
echo.
echo %BLUE%[INFO]%NC% 检查生成的文件...

set "EXE_FOUND=0"

:: 检查不同可能的路径
set "POSSIBLE_PATHS=bin Release\bin Debug\bin ."

for %%p in (%POSSIBLE_PATHS%) do (
    if exist "%%p\body_controller_web_server.exe" (
        echo %GREEN%[FOUND]%NC% REST API服务器: %%p\body_controller_web_server.exe
        set "EXE_FOUND=1"
        set "API_SERVER_PATH=%%p\body_controller_web_server.exe"
    )
    if exist "%%p\body_controller_websocket_server.exe" (
        echo %GREEN%[FOUND]%NC% WebSocket服务器: %%p\body_controller_websocket_server.exe
        set "EXE_FOUND=1"
        set "WS_SERVER_PATH=%%p\body_controller_websocket_server.exe"
    )
)

if !EXE_FOUND! == 0 (
    echo %YELLOW%[WARNING]%NC% 未找到可执行文件，列出build目录内容:
    dir /s *.exe
)

echo.
echo ========================================
echo 编译完成！
echo ========================================
echo.
echo 下一步操作:
echo 1. 启动后端服务: scripts\start_backend_windows.bat
echo 2. 启动前端界面: scripts\start_frontend.bat
echo 3. 运行测试: scripts\test_integration_windows.bat
echo.
echo 按任意键退出...
pause >nul
