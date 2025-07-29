#!/bin/bash
# SM3优化项目构建脚本
# 支持多种编译配置和目标平台

set -e  # 遇到错误时退出

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# 项目配置
PROJECT_NAME="SM3-Optimization"
BUILD_DIR="build"
SRC_DIR="src"
TEST_DIR="tests"
BENCHMARK_DIR="benchmarks"

# 编译器配置
CC=${CC:-gcc}
CXX=${CXX:-g++}

# 默认编译选项
CFLAGS_BASE="-std=c99 -Wall -Wextra -I${SRC_DIR}"
CFLAGS_DEBUG="-g -O0 -DDEBUG"
CFLAGS_RELEASE="-O3 -DNDEBUG -march=native"
CFLAGS_SIMD="-mavx2 -mfma"

# 链接选项
LDFLAGS="-lm"

# 函数：打印带颜色的消息
print_message() {
    local color=$1
    local message=$2
    echo -e "${color}[$(date '+%H:%M:%S')] ${message}${NC}"
}

print_info() {
    print_message $BLUE "$1"
}

print_success() {
    print_message $GREEN "$1"
}

print_warning() {
    print_message $YELLOW "$1"
}

print_error() {
    print_message $RED "$1"
}

# 函数：检查依赖
check_dependencies() {
    print_info "检查构建依赖..."
    
    # 检查编译器
    if ! command -v $CC &> /dev/null; then
        print_error "编译器 $CC 未找到"
        exit 1
    fi
    
    print_info "使用编译器: $CC $(${CC} --version | head -1)"
    
    # 检查CPU特性
    if command -v lscpu &> /dev/null; then
        print_info "CPU特性检测:"
        if lscpu | grep -q "avx2"; then
            print_success "  - AVX2: 支持"
        else
            print_warning "  - AVX2: 不支持"
        fi
        
        if lscpu | grep -q "fma"; then
            print_success "  - FMA: 支持"
        else
            print_warning "  - FMA: 不支持"
        fi
    fi
    
    echo
}

# 函数：清理构建目录
clean_build() {
    print_info "清理构建目录..."
    rm -rf $BUILD_DIR
    mkdir -p $BUILD_DIR
    print_success "构建目录已清理"
    echo
}

# 函数：编译基础实现
build_basic() {
    print_info "编译基础SM3实现..."
    
    local target="$BUILD_DIR/sm3_basic"
    local sources="$SRC_DIR/basic/sm3_basic.c"
    local cflags="$CFLAGS_BASE $1"
    
    $CC $cflags $sources -o $target $LDFLAGS
    
    if [[ -f $target ]]; then
        print_success "基础实现编译完成: $target"
    else
        print_error "基础实现编译失败"
        exit 1
    fi
}

# 函数：编译SIMD实现
build_simd() {
    print_info "编译SIMD SM3实现..."
    
    local target="$BUILD_DIR/sm3_simd"
    local sources="$SRC_DIR/simd/sm3_simd.c $SRC_DIR/basic/sm3_basic.c"
    local cflags="$CFLAGS_BASE $CFLAGS_SIMD $1"
    
    $CC $cflags $sources -o $target $LDFLAGS
    
    if [[ -f $target ]]; then
        print_success "SIMD实现编译完成: $target"
    else
        print_error "SIMD实现编译失败"
        exit 1
    fi
}

# 函数：编译优化实现
build_optimized() {
    print_info "编译优化SM3实现..."
    
    local target="$BUILD_DIR/sm3_optimized"
    local sources="$SRC_DIR/optimized/sm3_optimized.c $SRC_DIR/basic/sm3_basic.c"
    local cflags="$CFLAGS_BASE $1"
    
    $CC $cflags $sources -o $target $LDFLAGS
    
    if [[ -f $target ]]; then
        print_success "优化实现编译完成: $target"
    else
        print_error "优化实现编译失败"
        exit 1
    fi
}

# 函数：编译安全分析工具
build_security() {
    print_info "编译安全分析工具..."
    
    # Length-extension attack
    local le_target="$BUILD_DIR/sm3_length_extension"
    local le_sources="$SRC_DIR/security/sm3_length_extension.c $SRC_DIR/basic/sm3_basic.c"
    local cflags="$CFLAGS_BASE $1"
    
    $CC $cflags $le_sources -o $le_target $LDFLAGS
    
    if [[ -f $le_target ]]; then
        print_success "Length-extension攻击工具编译完成: $le_target"
    else
        print_error "Length-extension攻击工具编译失败"
        exit 1
    fi
}

# 函数：编译Merkle树实现
build_merkle() {
    print_info "编译Merkle树实现..."
    
    local target="$BUILD_DIR/sm3_merkle_tree"
    local sources="$SRC_DIR/applications/sm3_merkle_tree.c $SRC_DIR/basic/sm3_basic.c"
    local cflags="$CFLAGS_BASE $1"
    
    $CC $cflags $sources -o $target $LDFLAGS
    
    if [[ -f $target ]]; then
        print_success "Merkle树实现编译完成: $target"
    else
        print_error "Merkle树实现编译失败"
        exit 1
    fi
}

# 函数：编译测试程序
build_tests() {
    print_info "编译测试程序..."
    
    local target="$BUILD_DIR/test_sm3_comprehensive"
    local sources="$TEST_DIR/test_sm3_comprehensive.c $SRC_DIR/basic/sm3_basic.c"
    local sources="$sources $SRC_DIR/simd/sm3_simd.c $SRC_DIR/optimized/sm3_optimized.c"
    local sources="$sources $SRC_DIR/security/sm3_length_extension.c"
    local sources="$sources $SRC_DIR/applications/sm3_merkle_tree.c"
    local cflags="$CFLAGS_BASE $CFLAGS_SIMD $1"
    
    $CC $cflags $sources -o $target $LDFLAGS
    
    if [[ -f $target ]]; then
        print_success "综合测试程序编译完成: $target"
    else
        print_error "综合测试程序编译失败"
        exit 1
    fi
}

# 函数：编译基准测试
build_benchmarks() {
    print_info "编译基准测试程序..."
    
    local target="$BUILD_DIR/benchmark_comprehensive"
    local sources="$BENCHMARK_DIR/benchmark_comprehensive.c $SRC_DIR/basic/sm3_basic.c"
    local sources="$sources $SRC_DIR/simd/sm3_simd.c $SRC_DIR/optimized/sm3_optimized.c"
    local cflags="$CFLAGS_BASE $CFLAGS_SIMD $1"
    
    $CC $cflags $sources -o $target $LDFLAGS
    
    if [[ -f $target ]]; then
        print_success "基准测试程序编译完成: $target"
    else
        print_error "基准测试程序编译失败"
        exit 1
    fi
}

# 函数：运行测试
run_tests() {
    print_info "运行综合测试..."
    
    if [[ -f "$BUILD_DIR/test_sm3_comprehensive" ]]; then
        ./$BUILD_DIR/test_sm3_comprehensive
        if [[ $? -eq 0 ]]; then
            print_success "所有测试通过"
        else
            print_error "测试失败"
            exit 1
        fi
    else
        print_error "测试程序未找到，请先编译"
        exit 1
    fi
    
    echo
}

# 函数：运行基准测试
run_benchmarks() {
    print_info "运行性能基准测试..."
    
    if [[ -f "$BUILD_DIR/benchmark_comprehensive" ]]; then
        ./$BUILD_DIR/benchmark_comprehensive
        print_success "基准测试完成"
    else
        print_error "基准测试程序未找到，请先编译"
        exit 1
    fi
    
    echo
}

# 函数：显示帮助信息
show_help() {
    echo "SM3优化项目构建脚本"
    echo
    echo "用法: $0 [选项] [目标]"
    echo
    echo "选项:"
    echo "  -h, --help       显示此帮助信息"
    echo "  -c, --clean      清理构建目录"
    echo "  -d, --debug      使用调试模式编译"
    echo "  -r, --release    使用发布模式编译（默认）"
    echo "  -t, --test       编译后运行测试"
    echo "  -b, --benchmark  编译后运行基准测试"
    echo
    echo "目标:"
    echo "  all              编译所有组件（默认）"
    echo "  basic            仅编译基础实现"
    echo "  simd             仅编译SIMD实现"
    echo "  optimized        仅编译优化实现"
    echo "  security         仅编译安全分析工具"
    echo "  merkle           仅编译Merkle树实现"
    echo "  tests            仅编译测试程序"
    echo "  benchmarks       仅编译基准测试程序"
    echo
    echo "示例:"
    echo "  $0                    # 编译所有组件（发布模式）"
    echo "  $0 -d -t             # 调试模式编译并运行测试"
    echo "  $0 -c basic          # 清理后编译基础实现"
    echo "  $0 --benchmark       # 编译后运行基准测试"
    echo
}

# 主函数
main() {
    local build_mode="$CFLAGS_RELEASE"
    local clean_first=false
    local run_test=false
    local run_benchmark=false
    local targets=()
    
    # 解析命令行参数
    while [[ $# -gt 0 ]]; do
        case $1 in
            -h|--help)
                show_help
                exit 0
                ;;
            -c|--clean)
                clean_first=true
                shift
                ;;
            -d|--debug)
                build_mode="$CFLAGS_DEBUG"
                shift
                ;;
            -r|--release)
                build_mode="$CFLAGS_RELEASE"
                shift
                ;;
            -t|--test)
                run_test=true
                shift
                ;;
            -b|--benchmark)
                run_benchmark=true
                shift
                ;;
            all|basic|simd|optimized|security|merkle|tests|benchmarks)
                targets+=("$1")
                shift
                ;;
            *)
                print_error "未知选项: $1"
                show_help
                exit 1
                ;;
        esac
    done
    
    # 默认目标
    if [[ ${#targets[@]} -eq 0 ]]; then
        targets=("all")
    fi
    
    # 显示构建信息
    print_info "开始构建 $PROJECT_NAME"
    print_info "构建模式: $(echo $build_mode | grep -q DEBUG && echo "调试" || echo "发布")"
    print_info "目标: ${targets[*]}"
    echo
    
    # 检查依赖
    check_dependencies
    
    # 清理构建目录
    if [[ $clean_first == true ]]; then
        clean_build
    elif [[ ! -d $BUILD_DIR ]]; then
        mkdir -p $BUILD_DIR
    fi
    
    # 编译目标
    for target in "${targets[@]}"; do
        case $target in
            all)
                build_basic "$build_mode"
                build_simd "$build_mode"
                build_optimized "$build_mode"
                build_security "$build_mode"
                build_merkle "$build_mode"
                build_tests "$build_mode"
                build_benchmarks "$build_mode"
                ;;
            basic)
                build_basic "$build_mode"
                ;;
            simd)
                build_simd "$build_mode"
                ;;
            optimized)
                build_optimized "$build_mode"
                ;;
            security)
                build_security "$build_mode"
                ;;
            merkle)
                build_merkle "$build_mode"
                ;;
            tests)
                build_tests "$build_mode"
                ;;
            benchmarks)
                build_benchmarks "$build_mode"
                ;;
        esac
    done
    
    echo
    print_success "构建完成！"
    
    # 运行测试
    if [[ $run_test == true ]]; then
        run_tests
    fi
    
    # 运行基准测试
    if [[ $run_benchmark == true ]]; then
        run_benchmarks
    fi
    
    # 显示构建结果
    print_info "构建产物:"
    find $BUILD_DIR -type f -executable -exec echo "  - {}" \;
    echo
}

# 脚本入口点
if [[ "${BASH_SOURCE[0]}" == "${0}" ]]; then
    main "$@"
fi
