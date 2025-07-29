@echo off
REM SM3优化项目Windows构建脚本
REM 支持MSVC和MinGW编译器

setlocal EnableDelayedExpansion

:: 项目配置
set "PROJECT_NAME=SM3-Optimization"
set "BUILD_DIR=build"
set "SRC_DIR=src"
set "TEST_DIR=tests"
set "BENCHMARK_DIR=benchmarks"

:: 编译器检测
set "COMPILER="
set "CC="
set "CFLAGS_BASE="
set "LDFLAGS="

:: 颜色代码（Windows 10+）
set "COLOR_RED=[91m"
set "COLOR_GREEN=[92m"
set "COLOR_YELLOW=[93m"
set "COLOR_BLUE=[94m"
set "COLOR_RESET=[0m"

:: 函数：打印消息
:print_info
echo %COLOR_BLUE%[%time:~0,8%] %~1%COLOR_RESET%
goto :eof

:print_success
echo %COLOR_GREEN%[%time:~0,8%] %~1%COLOR_RESET%
goto :eof

:print_warning
echo %COLOR_YELLOW%[%time:~0,8%] %~1%COLOR_RESET%
goto :eof

:print_error
echo %COLOR_RED%[%time:~0,8%] %~1%COLOR_RESET%
goto :eof

:: 函数：检查编译器
:check_compiler
call :print_info "检查可用编译器..."

:: 检查MSVC
cl.exe >nul 2>&1
if !errorlevel! equ 0 (
    set "COMPILER=MSVC"
    set "CC=cl.exe"
    set "CFLAGS_BASE=/nologo /W4 /I%SRC_DIR%"
    set "CFLAGS_DEBUG=/Od /Zi /DDEBUG"
    set "CFLAGS_RELEASE=/O2 /DNDEBUG"
    set "CFLAGS_SIMD=/arch:AVX2"
    set "LDFLAGS="
    call :print_success "使用编译器: MSVC"
    goto :eof
)

:: 检查MinGW
gcc.exe --version >nul 2>&1
if !errorlevel! equ 0 (
    set "COMPILER=GCC"
    set "CC=gcc.exe"
    set "CFLAGS_BASE=-std=c99 -Wall -Wextra -I%SRC_DIR%"
    set "CFLAGS_DEBUG=-g -O0 -DDEBUG"
    set "CFLAGS_RELEASE=-O3 -DNDEBUG -march=native"
    set "CFLAGS_SIMD=-mavx2 -mfma"
    set "LDFLAGS=-lm"
    call :print_success "使用编译器: GCC (MinGW)"
    goto :eof
)

:: 检查Clang
clang.exe --version >nul 2>&1
if !errorlevel! equ 0 (
    set "COMPILER=CLANG"
    set "CC=clang.exe"
    set "CFLAGS_BASE=-std=c99 -Wall -Wextra -I%SRC_DIR%"
    set "CFLAGS_DEBUG=-g -O0 -DDEBUG"
    set "CFLAGS_RELEASE=-O3 -DNDEBUG -march=native"
    set "CFLAGS_SIMD=-mavx2 -mfma"
    set "LDFLAGS=-lm"
    call :print_success "使用编译器: Clang"
    goto :eof
)

call :print_error "未找到可用的C编译器"
echo 请安装以下编译器之一：
echo   - Microsoft Visual Studio (MSVC)
echo   - MinGW-w64
echo   - LLVM Clang
exit /b 1

:: 函数：清理构建目录
:clean_build
call :print_info "清理构建目录..."
if exist "%BUILD_DIR%" rd /s /q "%BUILD_DIR%"
md "%BUILD_DIR%" 2>nul
call :print_success "构建目录已清理"
goto :eof

:: 函数：编译源文件
:compile_file
set "target=%~1"
set "sources=%~2"
set "cflags=%~3"
set "description=%~4"

call :print_info "编译%description%..."

if "%COMPILER%"=="MSVC" (
    %CC% %cflags% %sources% /Fe:"%target%" %LDFLAGS%
) else (
    %CC% %cflags% %sources% -o "%target%" %LDFLAGS%
)

if exist "%target%" (
    call :print_success "%description%编译完成: %target%"
) else (
    call :print_error "%description%编译失败"
    exit /b 1
)
goto :eof

:: 函数：编译基础实现
:build_basic
set "target=%BUILD_DIR%\sm3_basic.exe"
set "sources=%SRC_DIR%\basic\sm3_basic.c"
set "cflags=%CFLAGS_BASE% %~1"
call :compile_file "%target%" "%sources%" "%cflags%" "基础SM3实现"
goto :eof

:: 函数：编译SIMD实现
:build_simd
set "target=%BUILD_DIR%\sm3_simd.exe"
set "sources=%SRC_DIR%\simd\sm3_simd.c %SRC_DIR%\basic\sm3_basic.c"
set "cflags=%CFLAGS_BASE% %CFLAGS_SIMD% %~1"
call :compile_file "%target%" "%sources%" "%cflags%" "SIMD SM3实现"
goto :eof

:: 函数：编译优化实现
:build_optimized
set "target=%BUILD_DIR%\sm3_optimized.exe"
set "sources=%SRC_DIR%\optimized\sm3_optimized.c %SRC_DIR%\basic\sm3_basic.c"
set "cflags=%CFLAGS_BASE% %~1"
call :compile_file "%target%" "%sources%" "%cflags%" "优化SM3实现"
goto :eof

:: 函数：编译安全分析工具
:build_security
set "target=%BUILD_DIR%\sm3_length_extension.exe"
set "sources=%SRC_DIR%\security\sm3_length_extension.c %SRC_DIR%\basic\sm3_basic.c"
set "cflags=%CFLAGS_BASE% %~1"
call :compile_file "%target%" "%sources%" "%cflags%" "Length-extension攻击工具"
goto :eof

:: 函数：编译Merkle树实现
:build_merkle
set "target=%BUILD_DIR%\sm3_merkle_tree.exe"
set "sources=%SRC_DIR%\applications\sm3_merkle_tree.c %SRC_DIR%\basic\sm3_basic.c"
set "cflags=%CFLAGS_BASE% %~1"
call :compile_file "%target%" "%sources%" "%cflags%" "Merkle树实现"
goto :eof

:: 函数：编译测试程序
:build_tests
set "target=%BUILD_DIR%\test_sm3_comprehensive.exe"
set "sources=%TEST_DIR%\test_sm3_comprehensive.c %SRC_DIR%\basic\sm3_basic.c"
set "sources=!sources! %SRC_DIR%\simd\sm3_simd.c %SRC_DIR%\optimized\sm3_optimized.c"
set "sources=!sources! %SRC_DIR%\security\sm3_length_extension.c"
set "sources=!sources! %SRC_DIR%\applications\sm3_merkle_tree.c"
set "cflags=%CFLAGS_BASE% %CFLAGS_SIMD% %~1"
call :compile_file "%target%" "!sources!" "%cflags%" "综合测试程序"
goto :eof

:: 函数：编译基准测试
:build_benchmarks
set "target=%BUILD_DIR%\benchmark_comprehensive.exe"
set "sources=%BENCHMARK_DIR%\benchmark_comprehensive.c %SRC_DIR%\basic\sm3_basic.c"
set "sources=!sources! %SRC_DIR%\simd\sm3_simd.c %SRC_DIR%\optimized\sm3_optimized.c"
set "cflags=%CFLAGS_BASE% %CFLAGS_SIMD% %~1"
call :compile_file "%target%" "!sources!" "%cflags%" "基准测试程序"
goto :eof

:: 函数：运行测试
:run_tests
call :print_info "运行综合测试..."
if exist "%BUILD_DIR%\test_sm3_comprehensive.exe" (
    "%BUILD_DIR%\test_sm3_comprehensive.exe"
    if !errorlevel! equ 0 (
        call :print_success "所有测试通过"
    ) else (
        call :print_error "测试失败"
        exit /b 1
    )
) else (
    call :print_error "测试程序未找到，请先编译"
    exit /b 1
)
goto :eof

:: 函数：运行基准测试
:run_benchmarks
call :print_info "运行性能基准测试..."
if exist "%BUILD_DIR%\benchmark_comprehensive.exe" (
    "%BUILD_DIR%\benchmark_comprehensive.exe"
    call :print_success "基准测试完成"
) else (
    call :print_error "基准测试程序未找到，请先编译"
    exit /b 1
)
goto :eof

:: 函数：显示帮助信息
:show_help
echo SM3优化项目Windows构建脚本
echo.
echo 用法: %~nx0 [选项] [目标]
echo.
echo 选项:
echo   /h, /help       显示此帮助信息
echo   /c, /clean      清理构建目录
echo   /d, /debug      使用调试模式编译
echo   /r, /release    使用发布模式编译（默认）
echo   /t, /test       编译后运行测试
echo   /b, /benchmark  编译后运行基准测试
echo.
echo 目标:
echo   all             编译所有组件（默认）
echo   basic           仅编译基础实现
echo   simd            仅编译SIMD实现
echo   optimized       仅编译优化实现
echo   security        仅编译安全分析工具
echo   merkle          仅编译Merkle树实现
echo   tests           仅编译测试程序
echo   benchmarks      仅编译基准测试程序
echo.
echo 示例:
echo   %~nx0                    # 编译所有组件（发布模式）
echo   %~nx0 /d /t             # 调试模式编译并运行测试
echo   %~nx0 /c basic          # 清理后编译基础实现
echo   %~nx0 /benchmark        # 编译后运行基准测试
echo.
goto :eof

:: 主函数
:main
set "build_mode=%CFLAGS_RELEASE%"
set "clean_first=false"
set "run_test=false"
set "run_benchmark=false"
set "targets="

:: 解析命令行参数
:parse_args
if "%~1"=="" goto :done_parsing
if /i "%~1"=="/h" goto :show_help_and_exit
if /i "%~1"=="/help" goto :show_help_and_exit
if /i "%~1"=="/c" set "clean_first=true" & shift & goto :parse_args
if /i "%~1"=="/clean" set "clean_first=true" & shift & goto :parse_args
if /i "%~1"=="/d" set "build_mode=%CFLAGS_DEBUG%" & shift & goto :parse_args
if /i "%~1"=="/debug" set "build_mode=%CFLAGS_DEBUG%" & shift & goto :parse_args
if /i "%~1"=="/r" set "build_mode=%CFLAGS_RELEASE%" & shift & goto :parse_args
if /i "%~1"=="/release" set "build_mode=%CFLAGS_RELEASE%" & shift & goto :parse_args
if /i "%~1"=="/t" set "run_test=true" & shift & goto :parse_args
if /i "%~1"=="/test" set "run_test=true" & shift & goto :parse_args
if /i "%~1"=="/b" set "run_benchmark=true" & shift & goto :parse_args
if /i "%~1"=="/benchmark" set "run_benchmark=true" & shift & goto :parse_args
if /i "%~1"=="all" set "targets=%targets% all" & shift & goto :parse_args
if /i "%~1"=="basic" set "targets=%targets% basic" & shift & goto :parse_args
if /i "%~1"=="simd" set "targets=%targets% simd" & shift & goto :parse_args
if /i "%~1"=="optimized" set "targets=%targets% optimized" & shift & goto :parse_args
if /i "%~1"=="security" set "targets=%targets% security" & shift & goto :parse_args
if /i "%~1"=="merkle" set "targets=%targets% merkle" & shift & goto :parse_args
if /i "%~1"=="tests" set "targets=%targets% tests" & shift & goto :parse_args
if /i "%~1"=="benchmarks" set "targets=%targets% benchmarks" & shift & goto :parse_args

call :print_error "未知选项: %~1"
goto :show_help_and_exit

:show_help_and_exit
call :show_help
exit /b 0

:done_parsing
:: 默认目标
if "%targets%"=="" set "targets=all"

:: 显示构建信息
call :print_info "开始构建 %PROJECT_NAME%"
echo %build_mode% | findstr /C:"DEBUG" >nul && (
    call :print_info "构建模式: 调试"
) || (
    call :print_info "构建模式: 发布"
)
call :print_info "目标:%targets%"
echo.

:: 检查编译器
call :check_compiler
if !errorlevel! neq 0 exit /b 1

:: 清理构建目录
if "%clean_first%"=="true" (
    call :clean_build
) else (
    if not exist "%BUILD_DIR%" md "%BUILD_DIR%" 2>nul
)

:: 编译目标
for %%t in (%targets%) do (
    if /i "%%t"=="all" (
        call :build_basic "%build_mode%"
        call :build_simd "%build_mode%"
        call :build_optimized "%build_mode%"
        call :build_security "%build_mode%"
        call :build_merkle "%build_mode%"
        call :build_tests "%build_mode%"
        call :build_benchmarks "%build_mode%"
    ) else if /i "%%t"=="basic" (
        call :build_basic "%build_mode%"
    ) else if /i "%%t"=="simd" (
        call :build_simd "%build_mode%"
    ) else if /i "%%t"=="optimized" (
        call :build_optimized "%build_mode%"
    ) else if /i "%%t"=="security" (
        call :build_security "%build_mode%"
    ) else if /i "%%t"=="merkle" (
        call :build_merkle "%build_mode%"
    ) else if /i "%%t"=="tests" (
        call :build_tests "%build_mode%"
    ) else if /i "%%t"=="benchmarks" (
        call :build_benchmarks "%build_mode%"
    )
)

echo.
call :print_success "构建完成！"

:: 运行测试
if "%run_test%"=="true" (
    call :run_tests
    if !errorlevel! neq 0 exit /b 1
)

:: 运行基准测试
if "%run_benchmark%"=="true" (
    call :run_benchmarks
)

:: 显示构建结果
call :print_info "构建产物:"
for %%f in ("%BUILD_DIR%\*.exe") do echo   - %%f
echo.

goto :eof

:: 脚本入口点
call :main %*
exit /b !errorlevel!
