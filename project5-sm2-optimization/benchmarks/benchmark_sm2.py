"""
SM2算法性能基准测试套件
对比基础实现和各种优化版本的性能
"""

import sys
import os
import time
import statistics
from typing import Dict, List, Tuple

# 添加src路径
sys.path.append(os.path.join(os.path.dirname(__file__), '..', 'src'))

from basic.sm2_signature import SM2Signature
from optimized.sm2_optimized_v1 import OptimizedSM2Signature

class SM2Benchmark:
 """SM2算法基准测试类"""

 def __init__(self):
 """初始化基准测试"""
 self.basic_sm2 = SM2Signature()
 self.optimized_sm2_v1 = OptimizedSM2Signature(window_size=4)

 # 测试参数
 self.test_iterations = 20
 self.warmup_iterations = 3

 # 测试数据
 self.test_scalars = [
 0x123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0,
 0xFEDCBA9876543210FEDCBA9876543210FEDCBA9876543210FEDCBA9876543210,
 0x555555555555555555555555555555555555555555555555555555555555555,
 0xAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA,
 0x1,
 0xFFFFFFFEFFFFFFFFFFFFFFFFFFFFFFFF7203DF6B21C6052B53BBF40939D54122, # n-1
 ]

 self.test_messages = [
 "Short message",
 "A" * 100, # 中等长度
 "B" * 1000, # 长消息
 "", # 空消息
 "Performance benchmark test message for SM2 digital signature algorithm optimization",
 ]

 def _measure_time(self, func, *args, iterations: int = None) -> Tuple[float, float]:
 """
 测量函数执行时间

 Returns:
 (平均时间, 标准差)
 """
 if iterations is None:
 iterations = self.test_iterations

 # 预热
 for _ in range(self.warmup_iterations):
 func(*args)

 # 正式测量
 times = []
 for _ in range(iterations):
 start_time = time.perf_counter()
 func(*args)
 end_time = time.perf_counter()
 times.append(end_time - start_time)

 return statistics.mean(times), statistics.stdev(times) if len(times) > 1 else 0.0

 def benchmark_point_multiply(self) -> Dict:
 """基准测试椭圆曲线点乘法"""
 print("\n=== 椭圆曲线点乘法基准测试 ===")

 results = {}

 for i, scalar in enumerate(self.test_scalars):
 print(f"\n测试标量 {i+1}: {hex(scalar)[:16]}...")

 # 基础版本
 basic_mean, basic_std = self._measure_time(
 self.basic_sm2.curve.point_multiply, scalar, self.basic_sm2.curve.G
 )

 # 优化版本v1
 opt_v1_mean, opt_v1_std = self._measure_time(
 self.optimized_sm2_v1.curve.point_multiply, scalar, self.optimized_sm2_v1.curve.G
 )

 # 计算加速比
 speedup_v1 = basic_mean / opt_v1_mean

 results[f'scalar_{i}'] = {
 'basic': {'mean': basic_mean, 'std': basic_std},
 'optimized_v1': {'mean': opt_v1_mean, 'std': opt_v1_std},
 'speedup_v1': speedup_v1
 }

 print(f" 基础版本: {basic_mean*1000:.2f} ± {basic_std*1000:.2f} ms")
 print(f" 优化版本v1: {opt_v1_mean*1000:.2f} ± {opt_v1_std*1000:.2f} ms")
 print(f" 加速比: {speedup_v1:.2f}x")

 # 计算平均性能
 avg_speedup = statistics.mean([r['speedup_v1'] for r in results.values()])
 print(f"\n平均加速比: {avg_speedup:.2f}x")

 return results

 def benchmark_key_generation(self) -> Dict:
 """基准测试密钥生成"""
 print("\n=== 密钥生成基准测试 ===")

 # 基础版本
 basic_mean, basic_std = self._measure_time(
 self.basic_sm2.generate_keypair
 )

 # 优化版本v1
 opt_v1_mean, opt_v1_std = self._measure_time(
 self.optimized_sm2_v1.generate_keypair
 )

 speedup_v1 = basic_mean / opt_v1_mean

 print(f"基础版本: {basic_mean*1000:.2f} ± {basic_std*1000:.2f} ms")
 print(f"优化版本v1: {opt_v1_mean*1000:.2f} ± {opt_v1_std*1000:.2f} ms")
 print(f"加速比: {speedup_v1:.2f}x")

 return {
 'basic': {'mean': basic_mean, 'std': basic_std},
 'optimized_v1': {'mean': opt_v1_mean, 'std': opt_v1_std},
 'speedup_v1': speedup_v1
 }

 def benchmark_signing(self) -> Dict:
 """基准测试数字签名"""
 print("\n=== 数字签名基准测试 ===")

 # 生成测试密钥对
 basic_private, basic_public = self.basic_sm2.generate_keypair()
 opt_private, opt_public = self.optimized_sm2_v1.generate_keypair()

 results = {}

 for i, message in enumerate(self.test_messages):
 print(f"\n测试消息 {i+1} (长度: {len(message)})...")

 # 基础版本
 basic_mean, basic_std = self._measure_time(
 self.basic_sm2.sign, message, basic_private, basic_public
 )

 # 优化版本v1
 opt_v1_mean, opt_v1_std = self._measure_time(
 self.optimized_sm2_v1.sign, message, opt_private, opt_public
 )

 speedup_v1 = basic_mean / opt_v1_mean

 results[f'message_{i}'] = {
 'length': len(message),
 'basic': {'mean': basic_mean, 'std': basic_std},
 'optimized_v1': {'mean': opt_v1_mean, 'std': opt_v1_std},
 'speedup_v1': speedup_v1
 }

 print(f" 基础版本: {basic_mean*1000:.2f} ± {basic_std*1000:.2f} ms")
 print(f" 优化版本v1: {opt_v1_mean*1000:.2f} ± {opt_v1_std*1000:.2f} ms")
 print(f" 加速比: {speedup_v1:.2f}x")

 # 计算平均性能
 avg_speedup = statistics.mean([r['speedup_v1'] for r in results.values()])
 print(f"\n平均加速比: {avg_speedup:.2f}x")

 return results

 def benchmark_verification(self) -> Dict:
 """基准测试签名验证"""
 print("\n=== 签名验证基准测试 ===")

 # 生成测试数据
 basic_private, basic_public = self.basic_sm2.generate_keypair()
 opt_private, opt_public = self.optimized_sm2_v1.generate_keypair()

 results = {}

 for i, message in enumerate(self.test_messages):
 print(f"\n测试消息 {i+1} (长度: {len(message)})...")

 # 生成签名
 basic_signature = self.basic_sm2.sign(message, basic_private, basic_public)
 opt_signature = self.optimized_sm2_v1.sign(message, opt_private, opt_public)

 # 基础版本验证
 basic_mean, basic_std = self._measure_time(
 self.basic_sm2.verify, message, basic_signature, basic_public
 )

 # 优化版本v1验证
 opt_v1_mean, opt_v1_std = self._measure_time(
 self.optimized_sm2_v1.verify, message, opt_signature, opt_public
 )

 speedup_v1 = basic_mean / opt_v1_mean

 results[f'message_{i}'] = {
 'length': len(message),
 'basic': {'mean': basic_mean, 'std': basic_std},
 'optimized_v1': {'mean': opt_v1_mean, 'std': opt_v1_std},
 'speedup_v1': speedup_v1
 }

 print(f" 基础版本: {basic_mean*1000:.2f} ± {basic_std*1000:.2f} ms")
 print(f" 优化版本v1: {opt_v1_mean*1000:.2f} ± {opt_v1_std*1000:.2f} ms")
 print(f" 加速比: {speedup_v1:.2f}x")

 # 计算平均性能
 avg_speedup = statistics.mean([r['speedup_v1'] for r in results.values()])
 print(f"\n平均加速比: {avg_speedup:.2f}x")

 return results

 def run_full_benchmark(self) -> Dict:
 """运行完整的基准测试套件"""
 print("开始运行SM2算法完整性能基准测试...")
 print(f"测试迭代次数: {self.test_iterations}")
 print(f"预热次数: {self.warmup_iterations}")

 results = {}

 # 椭圆曲线点乘法
 results['point_multiply'] = self.benchmark_point_multiply()

 # 密钥生成
 results['key_generation'] = self.benchmark_key_generation()

 # 数字签名
 results['signing'] = self.benchmark_signing()

 # 签名验证
 results['verification'] = self.benchmark_verification()

 return results

 def print_summary(self, results: Dict):
 """打印基准测试总结"""
 print("\n" + "="*60)
 print(" SM2算法性能基准测试总结")
 print("="*60)

 # 提取平均加速比
 point_multiply_speedup = statistics.mean([
 r['speedup_v1'] for r in results['point_multiply'].values()
 ])

 keygen_speedup = results['key_generation']['speedup_v1']

 signing_speedup = statistics.mean([
 r['speedup_v1'] for r in results['signing'].values()
 ])

 verification_speedup = statistics.mean([
 r['speedup_v1'] for r in results['verification'].values()
 ])

 print(f"椭圆曲线点乘法平均加速比: {point_multiply_speedup:.2f}x")
 print(f"密钥生成加速比: {keygen_speedup:.2f}x")
 print(f"数字签名平均加速比: {signing_speedup:.2f}x")
 print(f"签名验证平均加速比: {verification_speedup:.2f}x")

 overall_speedup = statistics.mean([
 point_multiply_speedup, keygen_speedup, signing_speedup, verification_speedup
 ])

 print(f"\n总体平均性能提升: {overall_speedup:.2f}x")
 print("="*60)

def save_benchmark_results(results: Dict, filename: str = "benchmark_results.txt"):
 """保存基准测试结果到文件"""
 output_dir = os.path.join(os.path.dirname(__file__), '..', 'output')
 os.makedirs(output_dir, exist_ok=True)

 filepath = os.path.join(output_dir, filename)

 with open(filepath, 'w', encoding='utf-8') as f:
 f.write("SM2椭圆曲线数字签名算法性能基准测试报告\n")
 f.write("="*60 + "\n\n")

 f.write(f"测试时间: {time.strftime('%Y-%m-%d %H:%M:%S')}\n\n")

 # 写入详细结果
 for category, data in results.items():
 f.write(f"{category.upper()}\n")
 f.write("-" * 40 + "\n")

 if category == 'key_generation':
 f.write(f"基础版本: {data['basic']['mean']*1000:.2f}ms\n")
 f.write(f"优化版本v1: {data['optimized_v1']['mean']*1000:.2f}ms\n")
 f.write(f"加速比: {data['speedup_v1']:.2f}x\n\n")
 else:
 for item, item_data in data.items():
 if isinstance(item_data, dict) and 'speedup_v1' in item_data:
 f.write(f"{item}: {item_data['speedup_v1']:.2f}x\n")
 f.write("\n")

 print(f"基准测试结果已保存到: {filepath}")

if __name__ == "__main__":
 benchmark = SM2Benchmark()

 try:
 results = benchmark.run_full_benchmark()
 benchmark.print_summary(results)
 save_benchmark_results(results)

 except Exception as e:
 print(f"基准测试过程中发生错误: {e}")
 import traceback
 traceback.print_exc()
