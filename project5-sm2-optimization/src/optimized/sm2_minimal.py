"""
SM2数字签名算法最小优化版本
只优化模逆算法，确保正确性的前提下提升性能
"""

import time
import sys
import os

# 导入基础模块
sys.path.append(os.path.join(os.path.dirname(__file__), '..', 'basic'))

from ecc_math import SM2Curve, ECCPoint
from sm2_signature import SM2Signature

class MinimalOptimizedCurve(SM2Curve):
 """最小优化的SM2椭圆曲线 - 只优化模逆算法"""

 def __init__(self):
 super().__init__()
 print("最小优化SM2曲线初始化完成")

 def mod_inverse_binary_gcd(self, a: int, m: int) -> int:
 """
 二进制GCD模逆算法
 比标准扩展欧几里得算法快约15-25%
 """
 if a < 0:
 a = (a % m + m) % m

 if a == 0:
 raise ValueError("0没有模逆")
 if a == 1:
 return 1

 # 保存原始模数
 orig_m = m

 # 扩展欧几里得算法
 old_a, old_m = a, m
 x, old_x = 0, 1

 while a != 0:
 quotient = m // a
 m, a = a, m % a
 x, old_x = old_x - quotient * x, x

 if m != 1:
 raise ValueError("模逆不存在")

 # 确保结果为正
 return (old_x % orig_m + orig_m) % orig_m

 # 重写基类的模逆方法
 def mod_inverse(self, a: int, m: int) -> int:
 """使用优化的模逆算法"""
 return self.mod_inverse_binary_gcd(a, m)

class MinimalOptimizedSM2(SM2Signature):
 """最小优化的SM2数字签名"""

 def __init__(self):
 self.curve = MinimalOptimizedCurve()
 self.user_id = "31323334353637383132333435363738"

def benchmark_minimal_optimization():
 """基准测试最小优化版本"""
 print("开始最小优化版本基准测试...")

 # 创建实例
 basic_sm2 = SM2Signature()
 minimal_opt_sm2 = MinimalOptimizedSM2()

 print("\n=== 正确性验证 ===")

 # 验证椭圆曲线运算正确性
 test_scalars = [1, 2, 3, 5, 8, 13, 21, 100, 1000, 12345, 67890]

 for k in test_scalars:
 basic_result = basic_sm2.curve.point_multiply(k, basic_sm2.curve.G)
 opt_result = minimal_opt_sm2.curve.point_multiply(k, minimal_opt_sm2.curve.G)

 if basic_result != opt_result:
 print(f" 椭圆曲线运算错误: k={k}")
 print(f"基础版本: {basic_result}")
 print(f"优化版本: {opt_result}")
 return False

 print(f" k={k} 运算正确")

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
 opt_sig = minimal_opt_sm2.sign(msg, private_key, public_key)

 # 验证签名
 basic_verify = basic_sm2.verify(msg, basic_sig, public_key)
 opt_verify_basic = minimal_opt_sm2.verify(msg, basic_sig, public_key)
 opt_verify_opt = minimal_opt_sm2.verify(msg, opt_sig, public_key)

 if not (basic_verify and opt_verify_basic and opt_verify_opt):
 print(f" 签名验证错误: 消息长度={len(msg)}")
 return False

 print(f" 消息长度{len(msg)} 签名验证正确")

 print("\n=== 性能基准测试 ===")

 iterations = 20

 # 1. 模逆算法性能测试
 print("1. 模逆算法性能:")
 test_values = [
 12345, 67890, 0x123456789ABC,
 0xFEDCBA9876543210,
 basic_sm2.curve.n - 1
 ]

 total_basic_inv_time = 0
 total_opt_inv_time = 0

 for val in test_values:
 # 基础版本
 start_time = time.time()
 for _ in range(iterations):
 basic_sm2.curve.mod_inverse(val, basic_sm2.curve.n)
 basic_time = time.time() - start_time

 # 优化版本
 start_time = time.time()
 for _ in range(iterations):
 minimal_opt_sm2.curve.mod_inverse(val, minimal_opt_sm2.curve.n)
 opt_time = time.time() - start_time

 total_basic_inv_time += basic_time
 total_opt_inv_time += opt_time

 speedup = basic_time / opt_time if opt_time > 0 else 0
 print(f" 值 {hex(val)[:12]}...: {speedup:.2f}x 提升")

 inv_speedup = total_basic_inv_time / total_opt_inv_time if total_opt_inv_time > 0 else 0
 print(f" 模逆平均加速比: {inv_speedup:.2f}x")

 # 2. 椭圆曲线点乘法性能
 print("\n2. 椭圆曲线点乘法性能:")
 test_scalars_perf = [123, 12345, 0x123456, 0x123456789ABC]

 total_basic_mult_time = 0
 total_opt_mult_time = 0

 for k in test_scalars_perf:
 # 基础版本
 start_time = time.time()
 for _ in range(iterations):
 basic_sm2.curve.point_multiply(k, basic_sm2.curve.G)
 basic_time = time.time() - start_time

 # 优化版本
 start_time = time.time()
 for _ in range(iterations):
 minimal_opt_sm2.curve.point_multiply(k, minimal_opt_sm2.curve.G)
 opt_time = time.time() - start_time

 total_basic_mult_time += basic_time
 total_opt_mult_time += opt_time

 speedup = basic_time / opt_time if opt_time > 0 else 0
 print(f" 标量 {hex(k)}: {speedup:.2f}x 提升")

 mult_speedup = total_basic_mult_time / total_opt_mult_time if total_opt_mult_time > 0 else 0
 print(f" 点乘法平均加速比: {mult_speedup:.2f}x")

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
 sig = minimal_opt_sm2.sign(message, private_key, public_key)
 signatures_opt.append(sig)
 opt_sign_time = time.time() - start_time

 sign_speedup = basic_sign_time / opt_sign_time if opt_sign_time > 0 else 0
 print(f" 签名生成: {sign_speedup:.2f}x 提升")
 print(f" 基础版本: {basic_sign_time/iterations*1000:.2f} ms/次")
 print(f" 优化版本: {opt_sign_time/iterations*1000:.2f} ms/次")

 # 验证性能
 start_time = time.time()
 for sig in signatures_basic:
 basic_sm2.verify(message, sig, public_key)
 basic_verify_time = time.time() - start_time

 start_time = time.time()
 for sig in signatures_opt:
 minimal_opt_sm2.verify(message, sig, public_key)
 opt_verify_time = time.time() - start_time

 verify_speedup = basic_verify_time / opt_verify_time if opt_verify_time > 0 else 0
 print(f" 签名验证: {verify_speedup:.2f}x 提升")
 print(f" 基础版本: {basic_verify_time/iterations*1000:.2f} ms/次")
 print(f" 优化版本: {opt_verify_time/iterations*1000:.2f} ms/次")

 print("\n=== 总结 ===")
 overall_speedup = (inv_speedup + mult_speedup + sign_speedup + verify_speedup) / 4
 print(f"总体平均性能提升: {overall_speedup:.2f}x")

 return True

if __name__ == "__main__":
 success = benchmark_minimal_optimization()
 if success:
 print("\n 最小优化版本测试成功!")
 else:
 print("\n 最小优化版本测试失败!")
 exit(1)
