"""
SM2数字签名算法保守优化版本
只添加简单的缓存机制，不修改任何核心算法
"""

import time
import sys
import os

# 导入基础模块
sys.path.append(os.path.join(os.path.dirname(__file__), '..', 'basic'))

from ecc_math import SM2Curve, ECCPoint
from sm2_signature import SM2Signature


class CachedOptimizedCurve(SM2Curve):
    """带缓存的SM2椭圆曲线"""
    
    def __init__(self):
        super().__init__()
        # 添加一些常用点的缓存
        self._point_cache = {}
        self._precompute_small_multiples()
        print("缓存优化SM2曲线初始化完成")
    
    def _precompute_small_multiples(self):
        """预计算小倍数点"""
        # 预计算2G, 3G, ..., 16G
        current = self.G
        self._point_cache[1] = current
        
        for i in range(2, 17):
            current = self.point_add(current, self.G)
            self._point_cache[i] = current
    
    def point_multiply(self, k: int, P: ECCPoint) -> ECCPoint:
        """
        标量乘法 k*P，使用缓存加速小倍数
        """
        if k == 0:
            return ECCPoint()  # 无穷远点
        if k == 1:
            return P
        
        # 如果是基点G的小倍数，使用缓存
        if P == self.G and k in self._point_cache:
            return self._point_cache[k]
        
        # 否则使用基类的方法
        return super().point_multiply(k, P)


class CachedOptimizedSM2(SM2Signature):
    """带缓存的SM2数字签名"""
    
    def __init__(self):
        self.curve = CachedOptimizedCurve()
        self.user_id = "31323334353637383132333435363738"


def benchmark_cached_optimization():
    """基准测试缓存优化版本"""
    print("开始缓存优化版本基准测试...")
    
    # 创建实例
    basic_sm2 = SM2Signature()
    cached_opt_sm2 = CachedOptimizedSM2()
    
    print("\n=== 正确性验证 ===")
    
    # 验证椭圆曲线运算正确性
    test_scalars = [1, 2, 3, 5, 8, 13, 16, 21, 100, 1000, 12345, 67890]
    
    for k in test_scalars:
        basic_result = basic_sm2.curve.point_multiply(k, basic_sm2.curve.G)
        opt_result = cached_opt_sm2.curve.point_multiply(k, cached_opt_sm2.curve.G)
        
        if basic_result != opt_result:
            print(f"✗ 椭圆曲线运算错误: k={k}")
            print(f"基础版本: {basic_result}")
            print(f"优化版本: {opt_result}")
            return False
        
        print(f"✓ k={k} 运算正确")
    
    # 验证数字签名功能
    test_messages = [
        "Hello SM2",
        "Test message",
        "A" * 100,
        "",
        "Long message " * 50
    ]
    
    for msg in test_messages:
        # 生成密钥对
        private_key, public_key = basic_sm2.generate_keypair()
        
        # 生成签名
        basic_sig = basic_sm2.sign(msg, private_key, public_key)
        opt_sig = cached_opt_sm2.sign(msg, private_key, public_key)
        
        # 验证签名
        basic_verify = basic_sm2.verify(msg, basic_sig, public_key)
        opt_verify_basic = cached_opt_sm2.verify(msg, basic_sig, public_key)
        opt_verify_opt = cached_opt_sm2.verify(msg, opt_sig, public_key)
        
        if not (basic_verify and opt_verify_basic and opt_verify_opt):
            print(f"✗ 签名验证错误: 消息长度={len(msg)}")
            return False
        
        print(f"✓ 消息长度{len(msg)} 签名验证正确")
    
    print("\n=== 性能基准测试 ===")
    
    iterations = 50
    
    # 1. 小倍数椭圆曲线点乘法性能
    print("1. 小倍数椭圆曲线点乘法性能:")
    small_scalars = [1, 2, 3, 5, 8, 13, 16]
    
    total_basic_small_time = 0
    total_opt_small_time = 0
    
    for k in small_scalars:
        # 基础版本
        start_time = time.time()
        for _ in range(iterations * 5):  # 更多迭代因为小数值很快
            basic_sm2.curve.point_multiply(k, basic_sm2.curve.G)
        basic_time = time.time() - start_time
        
        # 优化版本
        start_time = time.time()
        for _ in range(iterations * 5):
            cached_opt_sm2.curve.point_multiply(k, cached_opt_sm2.curve.G)
        opt_time = time.time() - start_time
        
        total_basic_small_time += basic_time
        total_opt_small_time += opt_time
        
        speedup = basic_time / opt_time if opt_time > 0 else 0
        print(f"  k={k}: {speedup:.2f}x 提升")
    
    small_speedup = total_basic_small_time / total_opt_small_time if total_opt_small_time > 0 else 0
    print(f"  小倍数平均加速比: {small_speedup:.2f}x")
    
    # 2. 大倍数椭圆曲线点乘法性能
    print("\n2. 大倍数椭圆曲线点乘法性能:")
    large_scalars = [123, 12345, 0x123456]
    
    total_basic_large_time = 0
    total_opt_large_time = 0
    
    for k in large_scalars:
        # 基础版本
        start_time = time.time()
        for _ in range(iterations):
            basic_sm2.curve.point_multiply(k, basic_sm2.curve.G)
        basic_time = time.time() - start_time
        
        # 优化版本
        start_time = time.time()
        for _ in range(iterations):
            cached_opt_sm2.curve.point_multiply(k, cached_opt_sm2.curve.G)
        opt_time = time.time() - start_time
        
        total_basic_large_time += basic_time
        total_opt_large_time += opt_time
        
        speedup = basic_time / opt_time if opt_time > 0 else 0
        print(f"  k={k}: {speedup:.2f}x 提升")
    
    large_speedup = total_basic_large_time / total_opt_large_time if total_opt_large_time > 0 else 1.0
    print(f"  大倍数平均加速比: {large_speedup:.2f}x")
    
    # 3. 数字签名性能
    print("\n3. 数字签名性能:")
    message = "Performance test message for SM2 digital signature"
    private_key, public_key = basic_sm2.generate_keypair()
    
    # 签名性能
    start_time = time.time()
    signatures_basic = []
    for _ in range(iterations):
        sig = basic_sm2.sign(message, private_key, public_key)
        signatures_basic.append(sig)
    basic_sign_time = time.time() - start_time
    
    start_time = time.time()
    signatures_opt = []
    for _ in range(iterations):
        sig = cached_opt_sm2.sign(message, private_key, public_key)
        signatures_opt.append(sig)
    opt_sign_time = time.time() - start_time
    
    sign_speedup = basic_sign_time / opt_sign_time if opt_sign_time > 0 else 1.0
    print(f"  签名生成: {sign_speedup:.2f}x 提升")
    print(f"    基础版本: {basic_sign_time/iterations*1000:.2f} ms/次")
    print(f"    优化版本: {opt_sign_time/iterations*1000:.2f} ms/次")
    
    # 验证性能
    start_time = time.time()
    for sig in signatures_basic:
        basic_sm2.verify(message, sig, public_key)
    basic_verify_time = time.time() - start_time
    
    start_time = time.time()
    for sig in signatures_opt:
        cached_opt_sm2.verify(message, sig, public_key)
    opt_verify_time = time.time() - start_time
    
    verify_speedup = basic_verify_time / opt_verify_time if opt_verify_time > 0 else 1.0
    print(f"  签名验证: {verify_speedup:.2f}x 提升")
    print(f"    基础版本: {basic_verify_time/iterations*1000:.2f} ms/次")
    print(f"    优化版本: {opt_verify_time/iterations*1000:.2f} ms/次")
    
    print("\n=== 总结 ===")
    overall_speedup = (small_speedup + large_speedup + sign_speedup + verify_speedup) / 4
    print(f"总体平均性能提升: {overall_speedup:.2f}x")
    print(f"小倍数运算特别优化: {small_speedup:.2f}x")
    
    if small_speedup > 1.1:  # 至少10%提升
        print("✓ 缓存优化在小倍数运算上有效")
    
    return True


if __name__ == "__main__":
    success = benchmark_cached_optimization()
    if success:
        print("\n✓ 缓存优化版本测试成功!")
    else:
        print("\n✗ 缓存优化版本测试失败!")
        exit(1)
