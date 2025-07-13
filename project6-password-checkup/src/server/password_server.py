"""
Password Checkup协议服务端实现
管理已泄露密码数据库并处理客户端请求
"""

import hashlib
import json
import secrets
from typing import List, Dict, Any, Set
import sys
import os

# 添加crypto模块路径
sys.path.append(os.path.join(os.path.dirname(__file__), '..', 'crypto'))

from elliptic_curve import PasswordCheckupCrypto, ECPoint

class PasswordCheckupServer:
 """Password Checkup协议服务端"""

 def __init__(self):
 self.crypto = PasswordCheckupCrypto()
 self.server_key = self.crypto.generate_server_key()
 self.compromised_db = set() # 已泄露密码哈希数据库
 self.load_compromised_passwords()
 print("Password Checkup服务端初始化完成")
 print(f"服务端密钥: {hex(self.server_key)[:16]}...")
 print(f"已加载 {len(self.compromised_db)} 个泄露密码")

 def load_compromised_passwords(self):
 """加载已泄露密码数据库"""
 # 模拟常见泄露密码
 common_passwords = [
 "123456",
 "password",
 "123456789",
 "12345678",
 "12345",
 "1234567",
 "qwerty",
 "abc123",
 "password123",
 "admin",
 "letmein",
 "welcome",
 "monkey",
 "1234567890",
 "dragon",
 "master",
 "666666",
 "123123",
 "111111",
 "000000"
 ]

 # 对每个密码进行哈希并添加到数据库
 salt = b'password_checkup_salt'
 for password in common_passwords:
 password_hash = self.crypto.hash_password(password, salt)
 self.compromised_db.add(password_hash)

 print(f"已加载 {len(common_passwords)} 个常见泄露密码到数据库")

 def add_compromised_password(self, password: str, salt: bytes = b'password_checkup_salt'):
 """添加泄露密码到数据库"""
 password_hash = self.crypto.hash_password(password, salt)
 self.compromised_db.add(password_hash)
 print(f"添加泄露密码到数据库（哈希: {password_hash[:8].hex()}...）")

 def process_client_request(self, request: Dict[str, Any]) -> Dict[str, Any]:
 """
 处理客户端密码检查请求

 Args:
 request: 客户端请求

 Returns:
 处理结果响应
 """
 session_id = request.get('session_id')
 blinded_element_hex = request.get('blinded_element')

 print(f"处理客户端请求，会话ID: {session_id}")

 # 解析盲化元素
 blinded_element_bytes = bytes.fromhex(blinded_element_hex)
 blinded_element = self.crypto.bytes_to_point(blinded_element_bytes)

 print(f"收到盲化元素: {blinded_element}")

 # 对盲化元素进行服务端处理
 processed_element = self.crypto.server_process(blinded_element, self.server_key)

 # 对数据库中的所有泄露密码进行相同处理
 processed_db_elements = []
 for password_hash in self.compromised_db:
 # 将密码哈希映射到椭圆曲线点
 try:
 db_point = self.crypto.curve.hash_to_curve(password_hash)
 # 用服务端密钥处理
 processed_db_point = self.crypto.server_process(db_point, self.server_key)
 processed_db_elements.append(processed_db_point)
 except ValueError:
 # 跳过无法映射的元素
 continue

 print(f"处理了数据库中的 {len(processed_db_elements)} 个元素")

 # 构造响应（包含处理后的客户端元素和数据库元素）
 response_elements = [processed_element] + processed_db_elements

 response = {
 'session_id': session_id,
 'processed_elements': [
 self.crypto.point_to_bytes(elem).hex()
 for elem in response_elements
 ],
 'server_key_hint': self.server_key, # 仅用于演示，实际不传输
 'version': '1.0',
 'status': 'success'
 }

 print(f"响应生成完成，包含 {len(response_elements)} 个处理后的元素")
 return response

 def get_statistics(self) -> Dict[str, Any]:
 """获取服务端统计信息"""
 return {
 'total_compromised_passwords': len(self.compromised_db),
 'server_key_id': hex(self.server_key)[:16] + '...',
 'database_version': '1.0'
 }

 def update_database(self, new_passwords: List[str]):
 """更新泄露密码数据库"""
 salt = b'password_checkup_salt'
 added_count = 0

 for password in new_passwords:
 password_hash = self.crypto.hash_password(password, salt)
 if password_hash not in self.compromised_db:
 self.compromised_db.add(password_hash)
 added_count += 1

 print(f"数据库更新完成，新增 {added_count} 个泄露密码")
 return added_count

 def export_database_summary(self) -> Dict[str, Any]:
 """导出数据库摘要信息"""
 # 计算数据库的统计信息
 hash_lengths = {}
 for password_hash in self.compromised_db:
 length = len(password_hash)
 hash_lengths[length] = hash_lengths.get(length, 0) + 1

 return {
 'total_entries': len(self.compromised_db),
 'hash_length_distribution': hash_lengths,
 'server_info': {
 'key_id': hex(self.server_key)[:16] + '...',
 'curve': 'P-256',
 'protocol_version': '1.0'
 }
 }

 def simulate_breach_update(self, breach_name: str, passwords: List[str]):
 """模拟数据泄露事件的数据库更新"""
 print(f"模拟处理数据泄露事件: {breach_name}")
 print(f"新增泄露密码数量: {len(passwords)}")

 added_count = self.update_database(passwords)

 print(f"处理完成，实际新增 {added_count} 个密码到数据库")
 print(f"数据库当前总计: {len(self.compromised_db)} 个泄露密码")

 return {
 'breach_name': breach_name,
 'new_passwords': len(passwords),
 'actually_added': added_count,
 'total_database_size': len(self.compromised_db)
 }

def demo_server():
 """服务端演示程序"""
 print("=== Password Checkup服务端演示 ===\n")

 server = PasswordCheckupServer()

 print("1. 服务端统计信息")
 print("-" * 40)
 stats = server.get_statistics()
 for key, value in stats.items():
 print(f"{key}: {value}")

 print(f"\n2. 数据库摘要信息")
 print("-" * 40)
 summary = server.export_database_summary()
 print(f"数据库总条目: {summary['total_entries']}")
 print(f"哈希长度分布: {summary['hash_length_distribution']}")
 print(f"服务端密钥ID: {summary['server_info']['key_id']}")
 print(f"使用曲线: {summary['server_info']['curve']}")

 print(f"\n3. 模拟数据泄露事件处理")
 print("-" * 40)

 # 模拟新的数据泄露
 new_breach_passwords = [
 "sunshine",
 "princess",
 "football",
 "charlie",
 "aa123456",
 "password1",
 "qwerty123"
 ]

 breach_result = server.simulate_breach_update("测试数据泄露2023", new_breach_passwords)
 print(f"泄露事件处理结果:")
 for key, value in breach_result.items():
 print(f" {key}: {value}")

 print(f"\n4. 处理客户端请求演示")
 print("-" * 40)

 # 模拟客户端请求
 from password_client import PasswordCheckupClient

 client = PasswordCheckupClient()
 test_password = "123456" # 已知泄露密码

 print(f"模拟检查密码: '{test_password}'")
 request = client.prepare_password_check(test_password)

 # 服务端处理请求
 response = server.process_client_request(request)
 print(f"服务端响应状态: {response['status']}")
 print(f"响应元素数量: {len(response['processed_elements'])}")

 # 客户端处理响应
 is_compromised = client.process_server_response(response)
 result = "已泄露" if is_compromised else "安全"
 print(f"最终检查结果: {result}")

 print(f"\n服务端演示完成！")

if __name__ == "__main__":
 demo_server()
