"""
SM2椭圆曲线数字签名算法优化实现 - 简化版本
主要优化: 快速模逆算法和简单预计算
"""

import time
from typing import Tuple
import sys
import os

# 添加基础模块路径
sys.path.append(os.path.join(os.path.dirname(__file__), '..', 'basic'))

from ecc_math import SM2Curve, ECCPoint
from sm2_signature import SM2Signature


class SimplifiedOptimizedCurve(SM2Curve):
    """简化的SM2椭圆曲线优化实现"""
    
    def __init__(self):
        super().__init__()
        # 简单的预计算基点倍数
        self._precompute_base_multiples()
    
    def _precompute_base_multiples(self):
        """预计算基点的小倍数"""
        print("预计算基点倍数...")
        start_time = time.time()
        
        self.base_multiples = {}
        current = self.G
        
        # 预计算 1G, 2G, 3G, ..., 16G
        for i in range(1, 17):
            self.base_multiples[i] = current
            if i < 16:
                current = self.point_add(current, self.G)
        
        end_time = time.time()
        print(f"预计算完成，耗时: {(end_time - start_time)*1000:.2f} ms")
    
    def mod_inverse_binary(self, a: int, m: int) -> int:
        """
        二进制扩展欧几里得算法 - 更快的模逆实现
        """
        if a < 0:
            a = (a % m + m) % m
        
        if a == 1:
            return 1
        if a == 0:
            raise ValueError("0没有模逆")
        
        # 二进制GCD算法
        original_m = m
        y, x, last_x, last_y = 0, 1, 1, 0
        
        while a != 0:
            quotient = m // a
            m, a = a, m % a
            x, last_x = last_x - quotient * x, x
            y, last_y = last_y - quotient * y, y
        
        if m != 1:
            raise ValueError("模逆不存在")
        
        return (last_x % original_m + original_m) % original_m
    
    def point_multiply_optimized(self, k: int, P: ECCPoint) -> ECCPoint:
        """优化的标量乘法"""
        if k == 0:
            return ECCPoint(is_infinity=True)
        
        if k < 0:
            k = -k
            P = ECCPoint(P.x, (-P.y) % self.p)
        
        # 如果是基点且k较小，使用预计算
        if P == self.G and k <= 16:
            return self.base_multiples[k]
        
        # 使用二进制方法，但进行一些优化
        return self._binary_multiply_optimized(k, P)
    
    def _binary_multiply_optimized(self, k: int, P: ECCPoint) -> ECCPoint:
        """优化的二进制标量乘法"""
        if k == 0:
            return ECCPoint(is_infinity=True)
        if k == 1:
            return P
        
        # 使用从右到左的二进制方法
        result = ECCPoint(is_infinity=True)
        addend = P
        
        while k > 0:
            if k & 1:
                result = self.point_add(result, addend)
            addend = self.point_double(addend)
            k >>= 1
        
        return result
    
    # 重写基类方法使用优化版本
    def mod_inverse(self, a: int, m: int) -> int:
        """使用优化的模逆算法"""
        return self.mod_inverse_binary(a, m)
    
    def point_multiply(self, k: int, P: ECCPoint) -> ECCPoint:
        """使用优化的标量乘法"""
        return self.point_multiply_optimized(k, P)


class SimplifiedOptimizedSM2(SM2Signature):
    """简化优化的SM2数字签名"""
    
    def __init__(self):
        # 使用优化的曲线
        self.curve = SimplifiedOptimizedCurve()
        self.user_id = "31323334353637383132333435363738"


def test_simplified_optimization():
    """测试简化优化版本"""
    print("开始测试简化优化版本...")
    
    # 创建实例
    from sm2_signature import SM2Signature
    basic_sm2 = SM2Signature()
    optimized_sm2 = SimplifiedOptimizedSM2()
    
    print("测试椭圆曲线点乘法正确性...")
    
    # 测试多个标量值
    test_scalars = [1, 2, 3, 7, 15, 16, 123, 12345]
    
    for k in test_scalars:
        basic_result = basic_sm2.curve.point_multiply(k, basic_sm2.curve.G)
        opt_result = optimized_sm2.curve.point_multiply(k, optimized_sm2.curve.G)
        
        if basic_result != opt_result:
            print(f"错误: k={k}时结果不一致")
            print(f"基础版本: {basic_result}")
            print(f"优化版本: {opt_result}")
            return False
        else:
            print(f"✓ k={k} 结果一致")
    
    print("所有测试通过！")
    
    # 性能测试
    print("\n性能对比测试:")
    test_k = 98765
    iterations = 10
    
    # 基础版本
    start_time = time.time()
    for _ in range(iterations):
        basic_sm2.curve.point_multiply(test_k, basic_sm2.curve.G)
    basic_time = (time.time() - start_time) / iterations
    
    # 优化版本
    start_time = time.time()
    for _ in range(iterations):
        optimized_sm2.curve.point_multiply(test_k, optimized_sm2.curve.G)
    opt_time = (time.time() - start_time) / iterations
    
    print(f"基础版本: {basic_time*1000:.2f} ms")
    print(f"优化版本: {opt_time*1000:.2f} ms")
    
    if opt_time > 0:
        speedup = basic_time / opt_time
        print(f"加速比: {speedup:.2f}x")
    
    return True


if __name__ == "__main__":
    success = test_simplified_optimization()
    if success:
        print("\n简化优化版本测试成功!")
    else:
        print("\n简化优化版本测试失败!")
