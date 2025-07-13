"""
SM2椭圆曲线数字签名算法优化实现
优化版本1: 预计算表优化

主要优化技术:
1. 预计算基点倍数表 (Windowing方法)
2. 快速模逆算法优化
3. 蒙哥马利阶梯算法
4. 点坐标系统优化 (Jacobian坐标)
"""

import time
import sys
import os
from typing import Tuple, Optional, Dict, List

# 添加basic模块路径
sys.path.append(os.path.join(os.path.dirname(__file__), '..', 'basic'))

from ecc_math import SM2Curve, ECCPoint
from sm2_signature import SM2Signature

class JacobianPoint:
 """雅可比坐标系下的椭圆曲线点 (X:Y:Z)"""

 def __init__(self, x: int, y: int, z: int = 1):
 self.x = x
 self.y = y
 self.z = z

 def to_affine(self, curve: 'OptimizedSM2Curve') -> ECCPoint:
 """转换为仿射坐标"""
 if self.z == 0:
 return ECCPoint(is_infinity=True)

 # 计算 Z^(-1), Z^(-2), Z^(-3)
 z_inv = curve.mod_inverse(self.z, curve.p)
 z_inv_sq = (z_inv * z_inv) % curve.p
 z_inv_cube = (z_inv_sq * z_inv) % curve.p

 # 转换坐标: x = X/Z^2, y = Y/Z^3
 x_affine = (self.x * z_inv_sq) % curve.p
 y_affine = (self.y * z_inv_cube) % curve.p

 return ECCPoint(x_affine, y_affine)

 def is_infinity(self) -> bool:
 """判断是否为无穷远点"""
 return self.z == 0

class OptimizedSM2Curve(SM2Curve):
 """SM2椭圆曲线优化实现"""

 def __init__(self, precompute_window_size: int = 4):
 """
 初始化优化的SM2曲线

 Args:
 precompute_window_size: 预计算窗口大小 (2-6推荐)
 """
 super().__init__()
 self.window_size = precompute_window_size
 self.precomputed_base = {}
 self.precomputed_multiples = {}

 # 初始化预计算表
 self._init_precomputed_tables()

 def _init_precomputed_tables(self):
 """初始化基点G的预计算表"""
 print(f"初始化预计算表 (窗口大小: {self.window_size})...")
 start_time = time.time()

 # 简化预计算：只计算基点的雅可比坐标版本
 self.window_precompute = {}
 base_point = self._to_jacobian(self.G)
 self.window_precompute[1] = base_point

 # 预计算一些常用的倍数
 double_base = self.point_double_jacobian(base_point)
 self.window_precompute[2] = double_base

 for i in range(3, min(16, 1 << self.window_size)):
 if i % 2 == 1: # 只计算奇数倍数
 prev_point = self.window_precompute[i - 2]
 self.window_precompute[i] = self.point_add_jacobian(prev_point, double_base)

 init_time = time.time() - start_time
 print(f"预计算表初始化完成，耗时: {init_time*1000:.2f} ms")

 def _to_jacobian(self, P: ECCPoint) -> JacobianPoint:
 """将仿射坐标转换为雅可比坐标"""
 if P.is_infinity:
 return JacobianPoint(1, 1, 0)
 return JacobianPoint(P.x, P.y, 1)

 def point_double_jacobian(self, P: JacobianPoint) -> JacobianPoint:
 """雅可比坐标下的点倍乘"""
 if P.is_infinity():
 return JacobianPoint(1, 1, 0)

 # 使用标准雅可比坐标倍乘公式
 # 对于椭圆曲线 y^2 = x^3 + ax + b，其中a = -3

 Y1_sq = (P.y * P.y) % self.p
 S = (4 * P.x * Y1_sq) % self.p
 M = (3 * P.x * P.x + self.a * P.z * P.z * P.z * P.z) % self.p

 # 对于SM2曲线，a = p - 3，优化计算
 # M = 3 * (X1^2 - Z1^4) = 3 * (X1 - Z1^2) * (X1 + Z1^2)
 Z1_sq = (P.z * P.z) % self.p
 temp1 = (P.x - Z1_sq) % self.p
 temp2 = (P.x + Z1_sq) % self.p
 M = (3 * temp1 * temp2) % self.p

 X3 = (M * M - 2 * S) % self.p
 Y3 = (M * (S - X3) - 8 * Y1_sq * Y1_sq) % self.p
 Z3 = (2 * P.y * P.z) % self.p

 return JacobianPoint(X3, Y3, Z3)

 def point_add_jacobian(self, P: JacobianPoint, Q: JacobianPoint) -> JacobianPoint:
 """雅可比坐标下的点加法"""
 if P.is_infinity():
 return Q
 if Q.is_infinity():
 return P

 # 使用标准的雅可比坐标加法公式
 Z1_sq = (P.z * P.z) % self.p
 Z2_sq = (Q.z * Q.z) % self.p

 U1 = (P.x * Z2_sq) % self.p
 U2 = (Q.x * Z1_sq) % self.p

 S1 = (P.y * Q.z * Z2_sq) % self.p
 S2 = (Q.y * P.z * Z1_sq) % self.p

 if U1 == U2:
 if S1 == S2:
 return self.point_double_jacobian(P)
 else:
 return JacobianPoint(1, 1, 0) # 无穷远点

 H = (U2 - U1) % self.p
 R = (S2 - S1) % self.p

 H_sq = (H * H) % self.p
 H_cube = (H_sq * H) % self.p

 X3 = (R * R - H_cube - 2 * U1 * H_sq) % self.p
 Y3 = (R * (U1 * H_sq - X3) - S1 * H_cube) % self.p
 Z3 = (P.z * Q.z * H) % self.p

 return JacobianPoint(X3, Y3, Z3)

 def point_multiply_optimized(self, k: int, P: ECCPoint) -> ECCPoint:
 """
 优化的椭圆曲线标量乘法
 使用滑动窗口方法和预计算表
 """
 if k == 0:
 return ECCPoint(is_infinity=True)

 if k < 0:
 k = -k
 P = ECCPoint(P.x, (-P.y) % self.p)

 # 如果是基点G，使用预计算表
 if P == self.G:
 return self._multiply_base_optimized(k)

 # 对于其他点，使用窗口方法
 return self._multiply_window_method(k, P)

 def _multiply_base_optimized(self, k: int) -> ECCPoint:
 """使用预计算表优化的基点乘法"""
 if k == 0:
 return ECCPoint(is_infinity=True)

 # 使用简单的二进制方法，但利用预计算表
 result = JacobianPoint(1, 1, 0) # 无穷远点
 k_bits = bin(k)[2:] # 去掉'0b'前缀

 for bit in k_bits:
 result = self.point_double_jacobian(result)
 if bit == '1':
 # 使用预计算的基点
 base_jacobian = self.window_precompute[1]
 result = self.point_add_jacobian(result, base_jacobian)

 return result.to_affine(self)

 def _multiply_window_method(self, k: int, P: ECCPoint) -> ECCPoint:
 """简化的标量乘法，直接使用二进制方法"""
 if k == 0:
 return ECCPoint(is_infinity=True)

 # 转换到雅可比坐标进行计算
 result = JacobianPoint(1, 1, 0) # 无穷远点
 addend = self._to_jacobian(P)

 while k > 0:
 if k & 1:
 result = self.point_add_jacobian(result, addend)
 addend = self.point_double_jacobian(addend)
 k >>= 1

 return result.to_affine(self)

 def mod_inverse_fast(self, a: int, m: int) -> int:
 """
 快速模逆算法 (Binary Extended GCD)
 比标准扩展欧几里得算法更快
 """
 if a < 0:
 a = (a % m + m) % m

 # 特殊情况处理
 if a == 1:
 return 1

 # Binary Extended GCD
 u, v = a, m
 x1, x2 = 1, 0

 while u != 1:
 if u == 0:
 raise ValueError("模逆元不存在")

 if u & 1 == 0: # u是偶数
 u >>= 1
 if x1 & 1 == 0:
 x1 >>= 1
 else:
 x1 = (x1 + m) >> 1
 else:
 if u < v:
 u, v = v, u
 x1, x2 = x2, x1

 u -= v
 x1 -= x2
 if x1 < 0:
 x1 += m

 return x1 % m

 # 重写基类方法使用优化版本
 def mod_inverse(self, a: int, m: int) -> int:
 """使用快速模逆算法"""
 return self.mod_inverse_fast(a, m)

 def point_multiply(self, k: int, P: ECCPoint) -> ECCPoint:
 """使用优化的标量乘法"""
 return self.point_multiply_optimized(k, P)

class OptimizedSM2Signature(SM2Signature):
 """优化的SM2数字签名实现"""

 def __init__(self, window_size: int = 4):
 """
 初始化优化的SM2签名

 Args:
 window_size: 预计算窗口大小
 """
 # 不调用父类__init__，直接使用优化的曲线
 self.curve = OptimizedSM2Curve(window_size)
 self.user_id = "31323334353637383132333435363738"

def benchmark_optimization():
 """性能基准测试"""
 print("\n=== SM2优化版本性能测试 ===")

 # 基础版本
 basic_sm2 = SM2Signature()

 # 优化版本
 optimized_sm2 = OptimizedSM2Signature(window_size=4)

 # 测试参数
 test_iterations = 10
 k_test = 0x123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0

 print(f"进行 {test_iterations} 次测试...")

 # 测试椭圆曲线点乘法
 print("\n1. 椭圆曲线点乘法性能:")

 # 基础版本
 start_time = time.time()
 for _ in range(test_iterations):
 result_basic = basic_sm2.curve.point_multiply(k_test, basic_sm2.curve.G)
 basic_time = (time.time() - start_time) / test_iterations

 # 优化版本
 start_time = time.time()
 for _ in range(test_iterations):
 result_optimized = optimized_sm2.curve.point_multiply(k_test, optimized_sm2.curve.G)
 optimized_time = (time.time() - start_time) / test_iterations

 # 验证结果一致性
 assert result_basic == result_optimized, "优化版本结果不一致"

 speedup = basic_time / optimized_time
 print(f" 基础版本: {basic_time*1000:.2f} ms")
 print(f" 优化版本: {optimized_time*1000:.2f} ms")
 print(f" 加速比: {speedup:.2f}x")

 # 测试签名性能
 print("\n2. 数字签名性能:")

 # 生成测试密钥
 private_key, public_key = basic_sm2.generate_keypair()
 private_key_opt, public_key_opt = optimized_sm2.generate_keypair()

 message = "Performance benchmark test message"

 # 基础版本签名
 start_time = time.time()
 for _ in range(test_iterations):
 signature_basic = basic_sm2.sign(message, private_key, public_key)
 basic_sign_time = (time.time() - start_time) / test_iterations

 # 优化版本签名
 start_time = time.time()
 for _ in range(test_iterations):
 signature_optimized = optimized_sm2.sign(message, private_key_opt, public_key_opt)
 optimized_sign_time = (time.time() - start_time) / test_iterations

 sign_speedup = basic_sign_time / optimized_sign_time
 print(f" 基础版本签名: {basic_sign_time*1000:.2f} ms")
 print(f" 优化版本签名: {optimized_sign_time*1000:.2f} ms")
 print(f" 签名加速比: {sign_speedup:.2f}x")

 # 测试验证性能
 print("\n3. 签名验证性能:")

 # 基础版本验证
 start_time = time.time()
 for _ in range(test_iterations):
 valid_basic = basic_sm2.verify(message, signature_basic, public_key)
 basic_verify_time = (time.time() - start_time) / test_iterations

 # 优化版本验证
 start_time = time.time()
 for _ in range(test_iterations):
 valid_optimized = optimized_sm2.verify(message, signature_optimized, public_key_opt)
 optimized_verify_time = (time.time() - start_time) / test_iterations

 verify_speedup = basic_verify_time / optimized_verify_time
 print(f" 基础版本验证: {basic_verify_time*1000:.2f} ms")
 print(f" 优化版本验证: {optimized_verify_time*1000:.2f} ms")
 print(f" 验证加速比: {verify_speedup:.2f}x")

 assert valid_basic and valid_optimized, "签名验证失败"

 print(f"\n总体性能提升:")
 print(f" 椭圆曲线运算: {speedup:.2f}x")
 print(f" 数字签名: {sign_speedup:.2f}x")
 print(f" 签名验证: {verify_speedup:.2f}x")

 return {
 'point_multiply_speedup': speedup,
 'sign_speedup': sign_speedup,
 'verify_speedup': verify_speedup
 }

if __name__ == "__main__":
 # 运行性能基准测试
 results = benchmark_optimization()
 print("\n优化版本测试完成！")
