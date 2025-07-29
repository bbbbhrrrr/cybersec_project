@echo off
echo SM4优化项目编译脚本
echo ====================

:: 创建build目录
if not exist build mkdir build
if not exist build\common mkdir build\common
if not exist build\basic mkdir build\basic
if not exist build\ttable mkdir build\ttable
if not exist build\simd mkdir build\simd
if not exist build\aesni mkdir build\aesni
if not exist build\gfni mkdir build\gfni
if not exist build\vprold mkdir build\vprold
if not exist build\gcm mkdir build\gcm

:: 设置编译选项
set CC=gcc
set CFLAGS=-Wall -Wextra -O3 -march=native -std=c99
set INCLUDES=-Isrc/common -Isrc/basic -Isrc/ttable -Isrc/simd -Isrc/aesni -Isrc/gfni -Isrc/vprold -Isrc/gcm

echo 编译通用模块...
%CC% %CFLAGS% %INCLUDES% -c src/common/sm4_common.c -o build/common/sm4_common.o

echo 编译基本实现...
%CC% %CFLAGS% %INCLUDES% -c src/basic/sm4_basic.c -o build/basic/sm4_basic.o

echo 编译T-table实现...
%CC% %CFLAGS% %INCLUDES% -c src/ttable/sm4_ttable.c -o build/ttable/sm4_ttable.o

echo 编译SIMD实现...
%CC% %CFLAGS% %INCLUDES% -msse4.1 -mavx2 -c src/simd/sm4_simd.c -o build/simd/sm4_simd.o

echo 编译AES-NI实现...
%CC% %CFLAGS% %INCLUDES% -maes -mpclmul -msse4.1 -mavx2 -c src/aesni/sm4_aesni.c -o build/aesni/sm4_aesni.o

echo 编译GFNI实现...
%CC% %CFLAGS% %INCLUDES% -mgfni -mavx512f -mavx512vl -c src/gfni/sm4_gfni.c -o build/gfni/sm4_gfni.o

echo 编译VPROLD实现...
%CC% %CFLAGS% %INCLUDES% -mavx512f -mavx512vl -c src/vprold/sm4_vprold.c -o build/vprold/sm4_vprold.o

echo 编译GCM实现...
%CC% %CFLAGS% %INCLUDES% -mpclmul -msse4.1 -mavx2 -c src/gcm/sm4_gcm.c -o build/gcm/sm4_gcm.o

echo 链接综合测试程序...
%CC% %CFLAGS% %INCLUDES% -maes -mpclmul -mgfni -mavx512f -mavx512vl -msse4.1 -mavx2 ^
    tests/test_sm4_optimization.c ^
    build/common/sm4_common.o build/basic/sm4_basic.o build/ttable/sm4_ttable.o ^
    build/simd/sm4_simd.o build/aesni/sm4_aesni.o build/gfni/sm4_gfni.o ^
    build/vprold/sm4_vprold.o build/gcm/sm4_gcm.o ^
    -o build/test_sm4_optimization.exe

echo 链接综合性能测试...
%CC% %CFLAGS% %INCLUDES% -maes -mpclmul -mgfni -mavx512f -mavx512vl -msse4.1 -mavx2 ^
    benchmarks/benchmark_comprehensive.c ^
    build/common/sm4_common.o build/basic/sm4_basic.o build/ttable/sm4_ttable.o ^
    build/simd/sm4_simd.o build/aesni/sm4_aesni.o build/gfni/sm4_gfni.o ^
    build/vprold/sm4_vprold.o build/gcm/sm4_gcm.o ^
    -o build/benchmark_comprehensive.exe

if %errorlevel% equ 0 (
    echo.
    echo 编译成功！
    echo 可执行文件位置：
    echo   build/test_sm4_optimization.exe - 综合功能测试
    echo   build/benchmark_comprehensive.exe - 综合性能测试
    echo.
    echo 运行测试：
    echo   build\test_sm4_optimization.exe
    echo   build\benchmark_comprehensive.exe
) else (
    echo.
    echo 编译失败，请检查错误信息
)

pause
