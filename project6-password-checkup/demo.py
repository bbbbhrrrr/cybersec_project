"""
Password Checkup协议完整演示程序
展示客户端-服务端交互的完整流程
"""

import sys
import os
import time

# 添加项目路径
sys.path.append(os.path.join(os.path.dirname(__file__), 'src', 'crypto'))
sys.path.append(os.path.join(os.path.dirname(__file__), 'src', 'client'))
sys.path.append(os.path.join(os.path.dirname(__file__), 'src', 'server'))

from elliptic_curve import PasswordCheckupCrypto, ECPoint
from password_client import PasswordCheckupClient
from password_server import PasswordCheckupServer

def print_section(title):
 """打印分节标题"""
 print(f"\n{'='*60}")
 print(f" {title}")
 print(f"{'='*60}")

def print_subsection(title):
 """打印子节标题"""
 print(f"\n{'-'*40}")
 print(f" {title}")
 print(f"{'-'*40}")

def demo_protocol_basics():
 """演示协议基础组件"""
 print_section("Protocol Basics - 协议基础组件演示")

 # 1. 椭圆曲线组件
 print_subsection("椭圆曲线密码学组件")
 crypto = PasswordCheckupCrypto()

 # 密钥生成
 private_key, public_key = crypto.curve.generate_keypair()
 print(f"生成密钥对:")
 print(f" 私钥: {hex(private_key)[:32]}...")
 print(f" 公钥: ({hex(public_key.x)[:32]}..., {hex(public_key.y)[:32]}...)")

 # 哈希到曲线
 test_data = b"test_password_hash"
 point = crypto.curve.hash_to_curve(test_data)
 print(f"哈希到曲线点: {point}")

 # 2. 盲化演示
 print_subsection("盲化操作演示")
 blind_factor = crypto.generate_blind_factor()
 blinded_point = crypto.blind_element(test_data, blind_factor)
 print(f"盲化因子: {hex(blind_factor)[:32]}...")
 print(f"盲化后点: {blinded_point}")

 # 服务端处理
 server_key = crypto.generate_server_key()
 processed_point = crypto.server_process(blinded_point, server_key)
 print(f"服务端处理后: {processed_point}")

 # 去盲化
 unblinded_point = crypto.unblind_element(processed_point, blind_factor)
 print(f"去盲化后: {unblinded_point}")

 return crypto

def demo_single_password_check():
 """演示单个密码检查流程"""
 print_section("Single Password Check - 单个密码检查演示")

 # 创建客户端和服务端
 client = PasswordCheckupClient()
 server = PasswordCheckupServer()

 # 测试密码
 test_passwords = [
 ("123456", "常见泄露密码"),
 ("password", "常见泄露密码"),
 ("MySecurePassword2023!", "强密码")
 ]

 for password, description in test_passwords:
 print_subsection(f"检查密码: '{password}' ({description})")

 # 步骤1: 客户端准备请求
 print("步骤1: 客户端准备密码检查请求")
 start_time = time.time()
 request = client.prepare_password_check(password)
 prep_time = time.time() - start_time
 print(f" 准备时间: {prep_time*1000:.2f} ms")
 print(f" 会话ID: {request['session_id']}")
 print(f" 盲化元素大小: {len(request['blinded_element'])} 字符")

 # 步骤2: 服务端处理请求
 print("步骤2: 服务端处理请求")
 start_time = time.time()
 response = server.process_client_request(request)
 process_time = time.time() - start_time
 print(f" 处理时间: {process_time*1000:.2f} ms")
 print(f" 响应状态: {response['status']}")
 print(f" 返回元素数量: {len(response['processed_elements'])}")

 # 步骤3: 客户端处理响应
 print("步骤3: 客户端处理响应并得出结论")
 start_time = time.time()
 is_compromised = client.process_server_response(response)
 verify_time = time.time() - start_time
 print(f" 验证时间: {verify_time*1000:.2f} ms")

 # 结果
 status = "已泄露" if is_compromised else "安全"
 total_time = prep_time + process_time + verify_time
 print(f" 检查结果: {status}")
 print(f" 总耗时: {total_time*1000:.2f} ms")

 # 密码强度分析
 strength = client.check_password_strength(password)
 print(f" 密码强度: {strength['strength_level']} (评分: {strength['strength_score']}/100)")

def demo_batch_password_check():
 """演示批量密码检查"""
 print_section("Batch Password Check - 批量密码检查演示")

 client = PasswordCheckupClient()

 # 批量测试密码
 batch_passwords = [
 "123456", "password", "qwerty", "abc123", "admin",
 "SecureP@ss1", "MyPassword123!", "ComplexP@ssw0rd2023",
 "letmein", "welcome", "monkey", "dragon"
 ]

 print(f"批量检查 {len(batch_passwords)} 个密码...")

 start_time = time.time()
 results = client.batch_check_passwords(batch_passwords)
 total_time = time.time() - start_time

 # 统计结果
 compromised_count = sum(1 for is_comp in results.values() if is_comp)
 safe_count = len(results) - compromised_count

 print(f"\n批量检查结果:")
 print(f" 总密码数: {len(batch_passwords)}")
 print(f" 已泄露: {compromised_count}")
 print(f" 安全: {safe_count}")
 print(f" 总耗时: {total_time:.2f} 秒")
 print(f" 平均每个: {total_time/len(batch_passwords)*1000:.2f} ms")

 # 详细结果
 print(f"\n详细结果:")
 for password, is_compromised in results.items():
 status = "已泄露" if is_compromised else "安全"
 strength = client.check_password_strength(password)
 print(f" '{password:15}': {status:4} (强度: {strength['strength_level']:2})")

def demo_server_operations():
 """演示服务端操作"""
 print_section("Server Operations - 服务端操作演示")

 server = PasswordCheckupServer()

 # 统计信息
 print_subsection("服务端统计信息")
 stats = server.get_statistics()
 for key, value in stats.items():
 print(f" {key}: {value}")

 # 数据库摘要
 print_subsection("数据库摘要")
 summary = server.export_database_summary()
 print(f" 总条目数: {summary['total_entries']}")
 print(f" 哈希长度分布: {summary['hash_length_distribution']}")
 print(f" 服务端信息:")
 for key, value in summary['server_info'].items():
 print(f" {key}: {value}")

 # 模拟数据泄露更新
 print_subsection("模拟数据泄露事件")
 new_breach_passwords = [
 "sunshine123", "princess", "football", "charlie",
 "iloveyou", "trustno1", "starwars", "montypython"
 ]

 breach_result = server.simulate_breach_update("模拟泄露2023", new_breach_passwords)
 print(f"泄露事件处理结果:")
 for key, value in breach_result.items():
 print(f" {key}: {value}")

def demo_security_analysis():
 """演示安全性分析"""
 print_section("Security Analysis - 安全性分析演示")

 # 密码强度分析
 print_subsection("密码强度分析")
 client = PasswordCheckupClient()

 test_passwords = [
 "123",
 "password",
 "Password1",
 "P@ssw0rd123",
 "VerySecureComplexP@ssw0rd2023!@#$%"
 ]

 print("密码强度评估:")
 for password in test_passwords:
 analysis = client.check_password_strength(password)
 print(f" '{password:30}': {analysis['strength_level']:3} "
 f"(评分: {analysis['strength_score']:3}/100, "
 f"熵值: {analysis['entropy_estimate']:6.1f} bits)")

 # 协议安全特性
 print_subsection("协议安全特性验证")
 crypto = PasswordCheckupCrypto()

 # 盲化因子唯一性
 factors = [crypto.generate_blind_factor() for _ in range(10)]
 unique_factors = len(set(factors))
 print(f" 盲化因子唯一性: {unique_factors}/10 (100%表示完全唯一)")

 # 服务端密钥安全性
 server1 = PasswordCheckupServer()
 server2 = PasswordCheckupServer()
 keys_different = server1.server_key != server2.server_key
 print(f" 服务端密钥独立性: {'通过' if keys_different else '失败'}")

 # 哈希一致性
 data = b"test_consistency"
 point1 = crypto.curve.hash_to_curve(data)
 point2 = crypto.curve.hash_to_curve(data)
 hash_consistent = point1 == point2
 print(f" 哈希一致性: {'通过' if hash_consistent else '失败'}")

def demo_performance_benchmarks():
 """性能基准测试"""
 print_section("Performance Benchmarks - 性能基准测试")

 crypto = PasswordCheckupCrypto()
 client = PasswordCheckupClient()

 # 椭圆曲线运算性能
 print_subsection("椭圆曲线运算性能")
 iterations = 100

 # 点加法性能
 G = crypto.curve.G
 start_time = time.time()
 for _ in range(iterations):
 crypto.curve.point_add(G, G)
 add_time = time.time() - start_time
 print(f" 点加法: {add_time/iterations*1000:.2f} ms/次 ({iterations} 次测试)")

 # 标量乘法性能
 start_time = time.time()
 for _ in range(iterations):
 crypto.curve.point_multiply(12345, G)
 mult_time = time.time() - start_time
 print(f" 标量乘法: {mult_time/iterations*1000:.2f} ms/次 ({iterations} 次测试)")

 # 哈希到曲线性能
 test_data = b"performance_test_data"
 start_time = time.time()
 for _ in range(iterations):
 crypto.curve.hash_to_curve(test_data)
 hash_time = time.time() - start_time
 print(f" 哈希到曲线: {hash_time/iterations*1000:.2f} ms/次 ({iterations} 次测试)")

 # 密码检查性能
 print_subsection("密码检查性能")
 test_password = "performance_test_password_123"

 # 单次密码检查
 start_time = time.time()
 request = client.prepare_password_check(test_password)
 prep_time = time.time() - start_time
 print(f" 请求准备: {prep_time*1000:.2f} ms")

 # 密码强度分析性能
 start_time = time.time()
 for _ in range(iterations):
 client.check_password_strength(test_password)
 strength_time = time.time() - start_time
 print(f" 强度分析: {strength_time/iterations*1000:.2f} ms/次 ({iterations} 次测试)")

def main():
 """主演示程序"""
 print("Password Checkup协议完整演示")
 print("基于Google Password Checkup论文实现")
 print("使用私有集合交集(PSI)技术保护用户隐私")

 try:
 # 基础组件演示
 demo_protocol_basics()

 # 单个密码检查演示
 demo_single_password_check()

 # 批量密码检查演示
 demo_batch_password_check()

 # 服务端操作演示
 demo_server_operations()

 # 安全性分析演示
 demo_security_analysis()

 # 性能基准测试
 demo_performance_benchmarks()

 print_section("演示完成")
 print("Password Checkup协议实现演示成功完成！")
 print("该实现展示了如何在保护用户隐私的前提下检查密码泄露。")

 except Exception as e:
 print(f"\n演示过程中发生错误: {e}")
 import traceback
 traceback.print_exc()

if __name__ == "__main__":
 main()
