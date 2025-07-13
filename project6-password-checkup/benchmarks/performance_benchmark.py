"""
Password Checkup协议性能基准测试
分析协议在不同场景下的性能表现
"""

import time
import statistics
import sys
import os
import json

# 添加项目路径
sys.path.append(os.path.join(os.path.dirname(__file__), '..', 'src', 'crypto'))
sys.path.append(os.path.join(os.path.dirname(__file__), '..', 'src', 'client'))
sys.path.append(os.path.join(os.path.dirname(__file__), '..', 'src', 'server'))

from elliptic_curve import PasswordCheckupCrypto, ECPoint
from password_client import PasswordCheckupClient
from password_server import PasswordCheckupServer


class PasswordCheckupBenchmark:
    """Password Checkup协议性能基准测试"""
    
    def __init__(self):
        self.crypto = PasswordCheckupCrypto()
        self.client = PasswordCheckupClient()
        self.server = PasswordCheckupServer()
        self.results = {}
        print("Password Checkup性能基准测试初始化完成")
    
    def benchmark_crypto_operations(self, iterations=1000):
        """测试基础密码学运算性能"""
        print(f"\n=== 基础密码学运算性能测试 ({iterations}次) ===")
        
        # 椭圆曲线点加法
        G = self.crypto.curve.G
        times = []
        for _ in range(iterations):
            start = time.perf_counter()
            self.crypto.curve.point_add(G, G)
            times.append(time.perf_counter() - start)
        
        avg_time = statistics.mean(times) * 1000
        std_dev = statistics.stdev(times) * 1000 if len(times) > 1 else 0
        print(f"椭圆曲线点加法: {avg_time:.3f}±{std_dev:.3f} ms")
        self.results['point_add'] = {'avg': avg_time, 'std': std_dev}
        
        # 椭圆曲线点倍乘
        times = []
        for _ in range(iterations):
            start = time.perf_counter()
            self.crypto.curve.point_double(G)
            times.append(time.perf_counter() - start)
        
        avg_time = statistics.mean(times) * 1000
        std_dev = statistics.stdev(times) * 1000 if len(times) > 1 else 0
        print(f"椭圆曲线点倍乘: {avg_time:.3f}±{std_dev:.3f} ms")
        self.results['point_double'] = {'avg': avg_time, 'std': std_dev}
        
        # 标量乘法 (不同标量大小)
        scalar_sizes = [8, 16, 32, 64, 128, 256]
        for bits in scalar_sizes:
            scalar = (1 << bits) - 1  # 全1的bits位数字
            times = []
            test_iterations = min(iterations, 100)  # 大标量时减少测试次数
            
            for _ in range(test_iterations):
                start = time.perf_counter()
                self.crypto.curve.point_multiply(scalar, G)
                times.append(time.perf_counter() - start)
            
            avg_time = statistics.mean(times) * 1000
            std_dev = statistics.stdev(times) * 1000 if len(times) > 1 else 0
            print(f"标量乘法({bits}位): {avg_time:.3f}±{std_dev:.3f} ms")
            self.results[f'scalar_mult_{bits}'] = {'avg': avg_time, 'std': std_dev}
        
        # 哈希到曲线
        test_data = b"benchmark_test_data"
        times = []
        for i in range(iterations):
            data = test_data + i.to_bytes(4, 'big')
            start = time.perf_counter()
            self.crypto.curve.hash_to_curve(data)
            times.append(time.perf_counter() - start)
        
        avg_time = statistics.mean(times) * 1000
        std_dev = statistics.stdev(times) * 1000 if len(times) > 1 else 0
        print(f"哈希到曲线: {avg_time:.3f}±{std_dev:.3f} ms")
        self.results['hash_to_curve'] = {'avg': avg_time, 'std': std_dev}
        
        # 模逆运算
        times = []
        for i in range(iterations):
            a = (12345 + i) % self.crypto.curve.n
            start = time.perf_counter()
            self.crypto.curve.mod_inverse(a, self.crypto.curve.n)
            times.append(time.perf_counter() - start)
        
        avg_time = statistics.mean(times) * 1000
        std_dev = statistics.stdev(times) * 1000 if len(times) > 1 else 0
        print(f"模逆运算: {avg_time:.3f}±{std_dev:.3f} ms")
        self.results['mod_inverse'] = {'avg': avg_time, 'std': std_dev}
    
    def benchmark_protocol_operations(self, iterations=100):
        """测试协议层面操作性能"""
        print(f"\n=== 协议操作性能测试 ({iterations}次) ===")
        
        # 密码哈希
        test_passwords = ["password123", "MySecurePassword!", "VeryLongPasswordForTesting123!@#"]
        
        for password in test_passwords:
            times = []
            for _ in range(iterations):
                start = time.perf_counter()
                self.crypto.hash_password(password)
                times.append(time.perf_counter() - start)
            
            avg_time = statistics.mean(times) * 1000
            std_dev = statistics.stdev(times) * 1000 if len(times) > 1 else 0
            print(f"密码哈希(长度{len(password)}): {avg_time:.3f}±{std_dev:.3f} ms")
            self.results[f'password_hash_len_{len(password)}'] = {'avg': avg_time, 'std': std_dev}
        
        # 盲化操作
        test_data = b"test_password_hash"
        times = []
        for _ in range(iterations):
            blind_factor = self.crypto.generate_blind_factor()
            start = time.perf_counter()
            self.crypto.blind_element(test_data, blind_factor)
            times.append(time.perf_counter() - start)
        
        avg_time = statistics.mean(times) * 1000
        std_dev = statistics.stdev(times) * 1000 if len(times) > 1 else 0
        print(f"盲化操作: {avg_time:.3f}±{std_dev:.3f} ms")
        self.results['blind_operation'] = {'avg': avg_time, 'std': std_dev}
        
        # 去盲化操作
        blinded_point = self.crypto.blind_element(test_data, 12345)
        times = []
        for _ in range(iterations):
            start = time.perf_counter()
            self.crypto.unblind_element(blinded_point, 12345)
            times.append(time.perf_counter() - start)
        
        avg_time = statistics.mean(times) * 1000
        std_dev = statistics.stdev(times) * 1000 if len(times) > 1 else 0
        print(f"去盲化操作: {avg_time:.3f}±{std_dev:.3f} ms")
        self.results['unblind_operation'] = {'avg': avg_time, 'std': std_dev}
    
    def benchmark_full_protocol(self, iterations=50):
        """测试完整协议流程性能"""
        print(f"\n=== 完整协议流程性能测试 ({iterations}次) ===")
        
        test_password = "BenchmarkTestPassword123!"
        
        # 客户端请求准备
        prep_times = []
        for _ in range(iterations):
            start = time.perf_counter()
            request = self.client.prepare_password_check(test_password)
            prep_times.append(time.perf_counter() - start)
        
        avg_prep = statistics.mean(prep_times) * 1000
        std_prep = statistics.stdev(prep_times) * 1000 if len(prep_times) > 1 else 0
        print(f"客户端请求准备: {avg_prep:.3f}±{std_prep:.3f} ms")
        self.results['client_prepare'] = {'avg': avg_prep, 'std': std_prep}
        
        # 服务端处理
        request = self.client.prepare_password_check(test_password)
        process_times = []
        for _ in range(iterations):
            start = time.perf_counter()
            response = self.server.process_client_request(request)
            process_times.append(time.perf_counter() - start)
        
        avg_process = statistics.mean(process_times) * 1000
        std_process = statistics.stdev(process_times) * 1000 if len(process_times) > 1 else 0
        print(f"服务端处理: {avg_process:.3f}±{std_process:.3f} ms")
        self.results['server_process'] = {'avg': avg_process, 'std': std_process}
        
        # 客户端响应处理
        verify_times = []
        for _ in range(iterations):
            # 为每次测试生成新的请求和响应
            test_request = self.client.prepare_password_check(test_password)
            test_response = self.server.process_client_request(test_request)
            start = time.perf_counter()
            self.client.process_server_response(test_response)
            verify_times.append(time.perf_counter() - start)
        
        avg_verify = statistics.mean(verify_times) * 1000
        std_verify = statistics.stdev(verify_times) * 1000 if len(verify_times) > 1 else 0
        print(f"客户端响应处理: {avg_verify:.3f}±{std_verify:.3f} ms")
        self.results['client_verify'] = {'avg': avg_verify, 'std': std_verify}
        
        # 总耗时
        total_avg = avg_prep + avg_process + avg_verify
        total_std = (std_prep**2 + std_process**2 + std_verify**2)**0.5
        print(f"协议总耗时: {total_avg:.3f}±{total_std:.3f} ms")
        self.results['total_protocol'] = {'avg': total_avg, 'std': total_std}
    
    def benchmark_scalability(self):
        """测试协议可扩展性"""
        print(f"\n=== 协议可扩展性测试 ===")
        
        # 不同数据库大小的影响
        original_db_size = len(self.server.compromised_db)
        db_sizes = [10, 20, 50, 100, 200]
        
        for size in db_sizes:
            if size <= original_db_size:
                continue
            
            # 扩展数据库到指定大小
            additional_passwords = [f"test_password_{i}" for i in range(size - original_db_size)]
            self.server.update_database(additional_passwords)
            
            # 测试性能
            test_password = "ScalabilityTestPassword"
            request = self.client.prepare_password_check(test_password)
            
            times = []
            for _ in range(10):  # 较少的迭代次数
                start = time.perf_counter()
                response = self.server.process_client_request(request)
                times.append(time.perf_counter() - start)
            
            avg_time = statistics.mean(times) * 1000
            std_time = statistics.stdev(times) * 1000 if len(times) > 1 else 0
            print(f"数据库大小{size}: {avg_time:.3f}±{std_time:.3f} ms")
            self.results[f'db_size_{size}'] = {'avg': avg_time, 'std': std_time}
        
        # 批量密码检查性能
        batch_sizes = [1, 5, 10, 20, 50]
        for batch_size in batch_sizes:
            passwords = [f"batch_test_password_{i}" for i in range(batch_size)]
            
            start = time.perf_counter()
            results = self.client.batch_check_passwords(passwords)
            total_time = time.perf_counter() - start
            
            avg_per_password = total_time / batch_size * 1000
            print(f"批量检查{batch_size}个密码: {avg_per_password:.3f} ms/个 (总计: {total_time*1000:.3f} ms)")
            self.results[f'batch_size_{batch_size}'] = {
                'avg_per_password': avg_per_password,
                'total_time': total_time * 1000
            }
    
    def benchmark_memory_usage(self):
        """分析内存使用情况"""
        print(f"\n=== 内存使用分析 ===")
        
        import sys
        
        # 椭圆曲线点大小
        point = ECPoint(12345, 67890)
        point_size = sys.getsizeof(point) + sys.getsizeof(point.x) + sys.getsizeof(point.y)
        print(f"椭圆曲线点: {point_size} 字节")
        
        # 序列化后大小
        serialized = self.crypto.point_to_bytes(self.crypto.curve.G)
        print(f"序列化点: {len(serialized)} 字节")
        
        # 数据库内存占用估算
        db_size = len(self.server.compromised_db)
        hash_size = 32  # SHA-256输出
        estimated_db_memory = db_size * hash_size
        print(f"数据库内存占用: ~{estimated_db_memory} 字节 ({estimated_db_memory/1024:.1f} KB)")
        
        # 请求/响应大小
        test_password = "MemoryTestPassword"
        request = self.client.prepare_password_check(test_password)
        response = self.server.process_client_request(request)
        
        request_size = len(json.dumps(request).encode('utf-8'))
        response_size = len(json.dumps(response).encode('utf-8'))
        
        print(f"请求大小: {request_size} 字节")
        print(f"响应大小: {response_size} 字节")
        
        self.results['memory_usage'] = {
            'point_size': point_size,
            'serialized_point_size': len(serialized),
            'database_memory': estimated_db_memory,
            'request_size': request_size,
            'response_size': response_size
        }
    
    def generate_report(self):
        """生成性能测试报告"""
        print(f"\n=== 性能测试报告生成 ===")
        
        report = {
            'timestamp': time.strftime('%Y-%m-%d %H:%M:%S'),
            'test_environment': {
                'python_version': sys.version,
                'platform': sys.platform,
            },
            'results': self.results
        }
        
        # 保存到文件
        output_dir = os.path.join(os.path.dirname(__file__), '..', 'output')
        os.makedirs(output_dir, exist_ok=True)
        
        report_file = os.path.join(output_dir, 'performance_benchmark_report.json')
        with open(report_file, 'w', encoding='utf-8') as f:
            json.dump(report, f, indent=2, ensure_ascii=False)
        
        # 生成人类可读的摘要
        summary_file = os.path.join(output_dir, 'performance_summary.txt')
        with open(summary_file, 'w', encoding='utf-8') as f:
            f.write("Password Checkup协议性能测试摘要\n")
            f.write("="*50 + "\n\n")
            
            f.write("关键性能指标:\n")
            f.write(f"- 完整协议耗时: {self.results.get('total_protocol', {}).get('avg', 0):.2f} ms\n")
            f.write(f"- 客户端准备: {self.results.get('client_prepare', {}).get('avg', 0):.2f} ms\n")
            f.write(f"- 服务端处理: {self.results.get('server_process', {}).get('avg', 0):.2f} ms\n")
            f.write(f"- 客户端验证: {self.results.get('client_verify', {}).get('avg', 0):.2f} ms\n\n")
            
            f.write("基础运算性能:\n")
            f.write(f"- 椭圆曲线点加法: {self.results.get('point_add', {}).get('avg', 0):.3f} ms\n")
            f.write(f"- 标量乘法(256位): {self.results.get('scalar_mult_256', {}).get('avg', 0):.3f} ms\n")
            f.write(f"- 哈希到曲线: {self.results.get('hash_to_curve', {}).get('avg', 0):.3f} ms\n\n")
            
            f.write("内存使用:\n")
            memory = self.results.get('memory_usage', {})
            f.write(f"- 椭圆曲线点: {memory.get('point_size', 0)} 字节\n")
            f.write(f"- 序列化点: {memory.get('serialized_point_size', 0)} 字节\n")
            f.write(f"- 数据库内存: {memory.get('database_memory', 0)} 字节\n")
            f.write(f"- 请求大小: {memory.get('request_size', 0)} 字节\n")
            f.write(f"- 响应大小: {memory.get('response_size', 0)} 字节\n")
        
        print(f"性能报告已保存到: {report_file}")
        print(f"性能摘要已保存到: {summary_file}")
    
    def run_all_benchmarks(self):
        """运行所有性能测试"""
        print("开始Password Checkup协议性能基准测试")
        print("="*60)
        
        try:
            self.benchmark_crypto_operations(iterations=500)
            self.benchmark_protocol_operations(iterations=100)
            self.benchmark_full_protocol(iterations=50)
            self.benchmark_scalability()
            self.benchmark_memory_usage()
            self.generate_report()
            
            print(f"\n{'='*60}")
            print("所有性能测试完成！")
            
        except Exception as e:
            print(f"性能测试过程中发生错误: {e}")
            import traceback
            traceback.print_exc()


def main():
    """主程序入口"""
    benchmark = PasswordCheckupBenchmark()
    benchmark.run_all_benchmarks()


if __name__ == "__main__":
    main()
