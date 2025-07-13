"""
简单的SM2优化版本测试
验证基本功能和性能
"""

import sys
import os
import time

# 添加src路径
sys.path.append(os.path.join(os.path.dirname(__file__), '..', 'src'))

def test_optimized_sm2():
    """测试优化版本的基本功能"""
    print("开始SM2优化版本测试...")
    
    # 导入模块
    from basic.sm2_signature import SM2Signature
    from optimized.sm2_optimized_v1 import OptimizedSM2Signature
    
    # 创建实例
    print("创建基础版本实例...")
    basic_sm2 = SM2Signature()
    print("基础SM2实例创建成功")
    
    print("创建优化版本实例...")
    optimized_sm2 = OptimizedSM2Signature(window_size=4)
    print("优化SM2实例创建成功")
    
    # 简单功能测试
    print("\n1. 测试密钥生成...")
    start_time = time.time()
    basic_private, basic_public = basic_sm2.generate_keypair()
    basic_keygen_time = time.time() - start_time
    print(f"基础版本密钥生成: {basic_keygen_time*1000:.2f} ms")
    
    start_time = time.time()
    opt_private, opt_public = optimized_sm2.generate_keypair()
    opt_keygen_time = time.time() - start_time
    print(f"优化版本密钥生成: {opt_keygen_time*1000:.2f} ms")
    
    # 简单性能测试
    print("\n2. 测试椭圆曲线点乘法...")
    test_scalar = 12345
    
    start_time = time.time()
    basic_result = basic_sm2.curve.point_multiply(test_scalar, basic_sm2.curve.G)
    basic_multiply_time = time.time() - start_time
    print(f"基础版本点乘法: {basic_multiply_time*1000:.2f} ms")
    
    start_time = time.time()
    opt_result = optimized_sm2.curve.point_multiply(test_scalar, optimized_sm2.curve.G)
    opt_multiply_time = time.time() - start_time
    print(f"优化版本点乘法: {opt_multiply_time*1000:.2f} ms")
    
    # 验证结果正确性
    assert basic_result == opt_result, "优化版本结果不正确"
    print("结果验证通过！")
    
    # 计算加速比
    if opt_multiply_time > 0:
        speedup = basic_multiply_time / opt_multiply_time
        print(f"椭圆曲线点乘法加速比: {speedup:.2f}x")
    
    print("\n3. 测试数字签名...")
    message = "Test message for SM2 optimization"
    
    start_time = time.time()
    basic_signature = basic_sm2.sign(message, basic_private, basic_public)
    basic_sign_time = time.time() - start_time
    print(f"基础版本签名: {basic_sign_time*1000:.2f} ms")
    
    start_time = time.time()
    opt_signature = optimized_sm2.sign(message, opt_private, opt_public)
    opt_sign_time = time.time() - start_time
    print(f"优化版本签名: {opt_sign_time*1000:.2f} ms")
    
    # 验证签名
    basic_valid = basic_sm2.verify(message, basic_signature, basic_public)
    opt_valid = optimized_sm2.verify(message, opt_signature, opt_public)
    
    assert basic_valid and opt_valid, "签名验证失败"
    print("签名验证通过！")
    
    if opt_sign_time > 0:
        sign_speedup = basic_sign_time / opt_sign_time
        print(f"数字签名加速比: {sign_speedup:.2f}x")
    
    print("\n测试完成！优化版本功能正常。")


if __name__ == "__main__":
    try:
        test_optimized_sm2()
    except Exception as e:
        print(f"测试过程中发生错误: {e}")
        import traceback
        traceback.print_exc()
