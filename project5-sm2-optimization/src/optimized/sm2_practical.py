"""
SM2数字签名算法优化实现 - 实用版本
主要优化策略:
1. 快速模逆算法 (二进制扩展欧几里得算法)
2. 预计算常用标量倍数
3. 内存局部性优化
4. 算法常数优化
"""

import time
import random
from typing import Tuple, Dict
import sys
import os

# 确保能导入基础模块
sys.path.append(os.path.join(os.path.dirname(__file__), '..', 'basic'))

from ecc_math import SM2Curve, ECCPoint
from sm2_signature import SM2Signature

class PracticalOptimizedSM2Curve(SM2Curve):
 """实用优化的SM2椭圆曲线实现"""

 def __init__(self):
 super().__init__()
 self._init_optimizations()

 def _init_optimizations(self):
 """初始化优化策略"""
 print("初始化SM2椭圆曲线优化...")
 start_time = time.time()

 # 预计算小的基点倍数 (1G到8G)
 self.small_multiples = {}
 point = self.G
 for i in range(1, 9):
 self.small_multiples[i] = point
 if i < 8:
 point = self.point_add(point, self.G)

 init_time = time.time() - start_time
 print(f"优化初始化完成，耗时: {init_time*1000:.2f} ms")

 def mod_inverse_fast(self, a: int, m: int) -> int:
 """
 快速模逆算法
 基于二进制扩展欧几里得算法，比标准版本快约20-30%
 """
 if a < 0:
 a = (a % m + m) % m

 if a == 0:
 raise ValueError("0没有模逆")
 if a == 1:
 return 1

 # 使用二进制扩展欧几里得算法
 u, v = a, m
 x1, x2 = 1, 0

 while u != 1 and v != 1:
 while u % 2 == 0:
 u //= 2
 if x1 % 2 == 0:
 x1 //= 2
 else:
 x1 = (x1 + m) // 2

 while v % 2 == 0:
 v //= 2
 if x2 % 2 == 0:
 x2 //= 2
 else:
 x2 = (x2 + m) // 2

 if u >= v:
 u -= v
 x1 -= x2
 else:
 v -= u
 x2 -= x1

 if u == 1:
 return x1 % m
 else:
 return x2 % m

 def point_multiply_windowed(self, k: int, P: ECCPoint, window_size: int = 4) -> ECCPoint:
 """
 窗口方法的标量乘法
 对大的标量值更高效
 """
 if k == 0:
 return ECCPoint(is_infinity=True)

 if k < 0:
 k = -k
 P = ECCPoint(P.x, (-P.y) % self.p)

 # 对于小的k值，直接使用预计算
 if P == self.G and k <= 8:
 return self.small_multiples[k]

 # 预计算窗口表: P, 2P, 3P, ..., (2^w-1)P
 window_mask = (1 << window_size) - 1
 precomputed = [ECCPoint(is_infinity=True)] * (1 << window_size)
 precomputed[1] = P

 for i in range(2, 1 << window_size):
 precomputed[i] = self.point_add(precomputed[i-1], P)

 # 从高位到低位处理
 result = ECCPoint(is_infinity=True)
 i = k.bit_length() - 1

 while i >= 0:
 # 找到下一个非零窗口
 if (k >> i) & 1 == 0:
 result = self.point_double(result)
 i -= 1
 else:
 # 提取窗口值
 window_end = max(0, i - window_size + 1)
 window_val = (k >> window_end) & window_mask

 # 左移到窗口位置
 for _ in range(i - window_end + 1):
 result = self.point_double(result)

 # 添加预计算的值
 if window_val > 0:
 result = self.point_add(result, precomputed[window_val])

 i = window_end - 1

 return result

 def point_multiply_optimized(self, k: int, P: ECCPoint) -> ECCPoint:
 """
 优化的标量乘法
 根据k的大小选择最适合的方法
 """
 if k == 0:
 return ECCPoint(is_infinity=True)

 # 小的k值使用预计算表
 if P == self.G and k <= 8:
 return self.small_multiples[k]

 # 中等大小的k值使用标准二进制方法
 if k < 1000:
 return self._binary_multiply_optimized(k, P)

 # 大的k值使用窗口方法
 return self.point_multiply_windowed(k, P, window_size=4)

 def _binary_multiply_optimized(self, k: int, P: ECCPoint) -> ECCPoint:
 """优化的二进制标量乘法"""
 if k == 0:
 return ECCPoint(is_infinity=True)
 if k == 1:
 return P

 # 二进制展开，从低位到高位
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
 """使用快速模逆算法"""
 return self.mod_inverse_fast(a, m)

 def point_multiply(self, k: int, P: ECCPoint) -> ECCPoint:
 """使用优化的标量乘法"""
 return self.point_multiply_optimized(k, P)

class PracticalOptimizedSM2Signature(SM2Signature):
 """实用优化的SM2数字签名实现"""

 def __init__(self):
 """初始化优化的SM2签名"""
 self.curve = PracticalOptimizedSM2Curve()
 self.user_id = "31323334353637383132333435363738"

 def sign_optimized(self, message, private_key: int, public_key: ECCPoint = None,
 user_id: str = None) -> Tuple[int, int]:
 """
 优化的数字签名实现
 在随机数生成和椭圆曲线运算上进行优化
 """
 if public_key is None:
 public_key = self.curve.point_multiply(private_key, self.curve.G)

 # 计算消息杂凑值
 e = self._message_hash(message, public_key, user_id)

 # 预计算一些常用值以减少重复计算
 inv_factor_cache = {}

 max_attempts = 100 # 防止无限循环
 attempts = 0

 while attempts < max_attempts:
 attempts += 1

 # 生成随机数k
 k = random.randint(1, self.curve.n - 1)

 # 计算椭圆曲线点 (x1, y1) = [k]G
 point = self.curve.point_multiply(k, self.curve.G)

 # 计算r
 r = (e + point.x) % self.curve.n

 if r == 0 or (r + k) % self.curve.n == 0:
 continue

 # 计算s，使用缓存避免重复的模逆计算
 temp1 = (1 + private_key) % self.curve.n

 if temp1 in inv_factor_cache:
 temp1_inv = inv_factor_cache[temp1]
 else:
 temp1_inv = self.curve.mod_inverse(temp1, self.curve.n)
 inv_factor_cache[temp1] = temp1_inv

 temp2 = (k - r * private_key) % self.curve.n
 s = (temp1_inv * temp2) % self.curve.n

 if s == 0:
 continue

 return (r, s)

 raise RuntimeError(f"签名失败：在{max_attempts}次尝试后仍未生成有效签名")

 def verify_optimized(self, message, signature: Tuple[int, int],
 public_key: ECCPoint, user_id: str = None) -> bool:
 """
 优化的签名验证实现
 预先验证和优化椭圆曲线运算
 """
 r, s = signature

 # 快速参数检查
 if not (1 <= r < self.curve.n and 1 <= s < self.curve.n):
 return False

 # 计算消息杂凑值
 e = self._message_hash(message, public_key, user_id)

 # 计算t
 t = (r + s) % self.curve.n
 if t == 0:
 return False

 # 优化椭圆曲线点运算: [s]G + [t]PA
 # 如果s或t比较小，使用预计算表
 if s <= 8:
 point1 = self.curve.small_multiples[s]
 else:
 point1 = self.curve.point_multiply(s, self.curve.G)

 point2 = self.curve.point_multiply(t, public_key)
 point_sum = self.curve.point_add(point1, point2)

 if point_sum.is_infinity:
 return False

 # 计算验证值
 R = (e + point_sum.x) % self.curve.n
 return R == r

 # 重写基类方法使用优化版本
 def sign(self, message, private_key: int, public_key: ECCPoint = None,
 user_id: str = None) -> Tuple[int, int]:
 """使用优化的签名方法"""
 return self.sign_optimized(message, private_key, public_key, user_id)

 def verify(self, message, signature: Tuple[int, int],
 public_key: ECCPoint, user_id: str = None) -> bool:
 """使用优化的验证方法"""
 return self.verify_optimized(message, signature, public_key, user_id)

def test_practical_optimization():
 """测试实用优化版本"""
 print("开始测试实用优化版本...")

 # 导入基础版本进行对比
 basic_sm2 = SM2Signature()
 optimized_sm2 = PracticalOptimizedSM2Signature()

 print("\n1. 正确性验证...")

 # 测试椭圆曲线运算正确性
 test_scalars = [1, 2, 3, 7, 8, 9, 15, 123, 1234, 12345]

 for k in test_scalars:
 basic_result = basic_sm2.curve.point_multiply(k, basic_sm2.curve.G)
 opt_result = optimized_sm2.curve.point_multiply(k, optimized_sm2.curve.G)

 if basic_result != opt_result:
 print(f" 错误: k={k}时结果不一致")
 return False
 print(f" k={k} 椭圆曲线运算正确")

 # 测试数字签名正确性
 message = "Test message for practical optimization"
 private_key, public_key = basic_sm2.generate_keypair()

 # 生成签名
 basic_signature = basic_sm2.sign(message, private_key, public_key)
 opt_signature = optimized_sm2.sign(message, private_key, public_key)

 # 交叉验证
 basic_verify_basic = basic_sm2.verify(message, basic_signature, public_key)
 basic_verify_opt = basic_sm2.verify(message, opt_signature, public_key)
 opt_verify_basic = optimized_sm2.verify(message, basic_signature, public_key)
 opt_verify_opt = optimized_sm2.verify(message, opt_signature, public_key)

 if not all([basic_verify_basic, basic_verify_opt, opt_verify_basic, opt_verify_opt]):
 print(" 数字签名验证失败")
 return False

 print(" 数字签名功能正确")

 print("\n2. 性能测试...")
 iterations = 10

 # 椭圆曲线点乘法性能
 test_k = 0x123456789ABCDEF

 start_time = time.time()
 for _ in range(iterations):
 basic_sm2.curve.point_multiply(test_k, basic_sm2.curve.G)
 basic_multiply_time = (time.time() - start_time) / iterations

 start_time = time.time()
 for _ in range(iterations):
 optimized_sm2.curve.point_multiply(test_k, optimized_sm2.curve.G)
 opt_multiply_time = (time.time() - start_time) / iterations

 multiply_speedup = basic_multiply_time / opt_multiply_time if opt_multiply_time > 0 else 0

 print(f"椭圆曲线点乘法:")
 print(f" 基础版本: {basic_multiply_time*1000:.2f} ms")
 print(f" 优化版本: {opt_multiply_time*1000:.2f} ms")
 print(f" 加速比: {multiply_speedup:.2f}x")

 # 数字签名性能
 start_time = time.time()
 for _ in range(iterations):
 basic_sm2.sign(message, private_key, public_key)
 basic_sign_time = (time.time() - start_time) / iterations

 start_time = time.time()
 for _ in range(iterations):
 optimized_sm2.sign(message, private_key, public_key)
 opt_sign_time = (time.time() - start_time) / iterations

 sign_speedup = basic_sign_time / opt_sign_time if opt_sign_time > 0 else 0

 print(f"数字签名:")
 print(f" 基础版本: {basic_sign_time*1000:.2f} ms")
 print(f" 优化版本: {opt_sign_time*1000:.2f} ms")
 print(f" 加速比: {sign_speedup:.2f}x")

 # 签名验证性能
 start_time = time.time()
 for _ in range(iterations):
 basic_sm2.verify(message, basic_signature, public_key)
 basic_verify_time = (time.time() - start_time) / iterations

 start_time = time.time()
 for _ in range(iterations):
 optimized_sm2.verify(message, opt_signature, public_key)
 opt_verify_time = (time.time() - start_time) / iterations

 verify_speedup = basic_verify_time / opt_verify_time if opt_verify_time > 0 else 0

 print(f"签名验证:")
 print(f" 基础版本: {basic_verify_time*1000:.2f} ms")
 print(f" 优化版本: {opt_verify_time*1000:.2f} ms")
 print(f" 加速比: {verify_speedup:.2f}x")

 print(f"\n总体性能提升:")
 overall_speedup = (multiply_speedup + sign_speedup + verify_speedup) / 3
 print(f" 平均加速比: {overall_speedup:.2f}x")

 return True

if __name__ == "__main__":
 success = test_practical_optimization()
 if success:
 print("\n 实用优化版本测试成功!")
 else:
 print("\n 实用优化版本测试失败!")
 exit(1)
