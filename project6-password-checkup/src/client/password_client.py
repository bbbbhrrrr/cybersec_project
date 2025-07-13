"""
Password Checkup协议客户端实现
用户端密码泄露检查功能
"""

import hashlib
import secrets
import json
from typing import List, Dict, Any, Optional
import sys
import os

# 添加crypto模块路径
sys.path.append(os.path.join(os.path.dirname(__file__), '..', 'crypto'))

from elliptic_curve import PasswordCheckupCrypto, ECPoint


class PasswordCheckupClient:
    """Password Checkup协议客户端"""
    
    def __init__(self):
        self.crypto = PasswordCheckupCrypto()
        self.session_data = {}
        print("Password Checkup客户端初始化完成")
    
    def prepare_password_check(self, password: str, salt: bytes = b'password_checkup_salt') -> Dict[str, Any]:
        """
        准备密码检查请求
        
        Args:
            password: 待检查的密码
            salt: 盐值
            
        Returns:
            包含盲化元素和会话信息的字典
        """
        print(f"准备检查密码（长度: {len(password)}）")
        
        # 1. 对密码进行哈希
        password_hash = self.crypto.hash_password(password, salt)
        print(f"密码哈希完成: {password_hash[:8].hex()}...")
        
        # 2. 生成盲化因子
        blind_factor = self.crypto.generate_blind_factor()
        print(f"生成盲化因子: {hex(blind_factor)[:16]}...")
        
        # 3. 对密码哈希进行盲化
        blinded_element = self.crypto.blind_element(password_hash, blind_factor)
        print(f"盲化完成: {blinded_element}")
        
        # 4. 保存会话数据
        session_id = secrets.token_hex(16)
        self.session_data[session_id] = {
            'password_hash': password_hash,
            'blind_factor': blind_factor,
            'blinded_element': blinded_element
        }
        
        # 5. 构造请求
        request = {
            'session_id': session_id,
            'blinded_element': self.crypto.point_to_bytes(blinded_element).hex(),
            'version': '1.0'
        }
        
        print(f"请求准备完成，会话ID: {session_id}")
        return request
    
    def process_server_response(self, response: Dict[str, Any]) -> bool:
        """
        处理服务端响应，判断密码是否泄露
        
        Args:
            response: 服务端响应
            
        Returns:
            True if password is compromised, False otherwise
        """
        session_id = response.get('session_id')
        if session_id not in self.session_data:
            raise ValueError("无效的会话ID")
        
        print(f"处理服务端响应，会话ID: {session_id}")
        
        # 获取会话数据
        session = self.session_data[session_id]
        
        # 解析服务端返回的处理结果
        processed_elements_hex = response.get('processed_elements', [])
        processed_elements = []
        
        for elem_hex in processed_elements_hex:
            elem_bytes = bytes.fromhex(elem_hex)
            elem_point = self.crypto.bytes_to_point(elem_bytes)
            processed_elements.append(elem_point)
        
        print(f"收到 {len(processed_elements)} 个处理后的元素")
        
        # 对每个处理后的元素进行去盲化
        unblinded_elements = []
        for processed_point in processed_elements:
            unblinded_point = self.crypto.unblind_element(processed_point, session['blind_factor'])
            unblinded_elements.append(unblinded_point)
        
        print(f"去盲化完成，得到 {len(unblinded_elements)} 个元素")
        
        # 计算用户密码的预期处理结果
        password_hash = session['password_hash']
        expected_point = self.crypto.curve.hash_to_curve(password_hash)
        server_key = response.get('server_key_hint')  # 实际实现中不会直接传输
        
        # 为了演示，我们简化处理：检查是否有匹配的元素
        is_compromised = False
        for unblinded_point in unblinded_elements:
            # 在实际实现中，这里需要更复杂的匹配机制
            if self._points_match(unblinded_point, expected_point):
                is_compromised = True
                break
        
        # 清理会话数据
        del self.session_data[session_id]
        
        result = "泄露" if is_compromised else "安全"
        print(f"密码检查结果: {result}")
        
        return is_compromised
    
    def _points_match(self, point1: ECPoint, point2: ECPoint) -> bool:
        """
        检查两个椭圆曲线点是否匹配
        实际实现中需要考虑服务端密钥处理
        """
        # 简化的匹配逻辑，实际实现更复杂
        return point1 == point2
    
    def batch_check_passwords(self, passwords: List[str]) -> Dict[str, bool]:
        """
        批量检查多个密码
        
        Args:
            passwords: 密码列表
            
        Returns:
            密码泄露状态字典
        """
        print(f"开始批量检查 {len(passwords)} 个密码")
        
        results = {}
        for i, password in enumerate(passwords):
            print(f"检查密码 {i+1}/{len(passwords)}")
            
            # 准备请求
            request = self.prepare_password_check(password)
            
            # 模拟服务端响应（在实际实现中需要网络通信）
            response = self._simulate_server_response(request)
            
            # 处理响应
            is_compromised = self.process_server_response(response)
            results[password] = is_compromised
        
        print(f"批量检查完成，共检查 {len(passwords)} 个密码")
        return results
    
    def _simulate_server_response(self, request: Dict[str, Any]) -> Dict[str, Any]:
        """
        模拟服务端响应（用于测试）
        实际实现中需要通过网络与服务端通信
        """
        # 导入服务端模块进行模拟
        sys.path.append(os.path.join(os.path.dirname(__file__), '..', 'server'))
        from password_server import PasswordCheckupServer
        
        # 创建服务端实例
        server = PasswordCheckupServer()
        
        # 处理请求
        return server.process_client_request(request)
    
    def check_password_strength(self, password: str) -> Dict[str, Any]:
        """
        检查密码强度（附加功能）
        
        Args:
            password: 待检查的密码
            
        Returns:
            密码强度分析结果
        """
        analysis = {
            'length': len(password),
            'has_uppercase': any(c.isupper() for c in password),
            'has_lowercase': any(c.islower() for c in password),
            'has_digits': any(c.isdigit() for c in password),
            'has_special': any(not c.isalnum() for c in password),
            'entropy_estimate': self._estimate_entropy(password)
        }
        
        # 计算强度评分
        score = 0
        if analysis['length'] >= 8:
            score += 20
        if analysis['length'] >= 12:
            score += 10
        if analysis['has_uppercase']:
            score += 15
        if analysis['has_lowercase']:
            score += 15
        if analysis['has_digits']:
            score += 15
        if analysis['has_special']:
            score += 15
        if analysis['entropy_estimate'] > 50:
            score += 10
        
        analysis['strength_score'] = min(score, 100)
        
        if score >= 80:
            analysis['strength_level'] = '强'
        elif score >= 60:
            analysis['strength_level'] = '中等'
        else:
            analysis['strength_level'] = '弱'
        
        return analysis
    
    def _estimate_entropy(self, password: str) -> float:
        """估算密码熵值"""
        charset_size = 0
        
        if any(c.islower() for c in password):
            charset_size += 26
        if any(c.isupper() for c in password):
            charset_size += 26
        if any(c.isdigit() for c in password):
            charset_size += 10
        if any(not c.isalnum() for c in password):
            charset_size += 32  # 估算特殊字符数量
        
        if charset_size == 0:
            return 0
        
        import math
        return len(password) * math.log2(charset_size)


def demo_client():
    """客户端演示程序"""
    print("=== Password Checkup客户端演示 ===\n")
    
    client = PasswordCheckupClient()
    
    # 测试密码列表
    test_passwords = [
        "123456",           # 常见弱密码
        "password",         # 常见弱密码
        "qwerty",          # 常见弱密码
        "P@ssw0rd2023!",   # 较强密码
        "MySecureP@ss123", # 较强密码
    ]
    
    print("1. 单个密码检查演示")
    print("-" * 40)
    
    for password in test_passwords[:2]:
        print(f"\n检查密码: '{password}'")
        
        # 准备检查
        request = client.prepare_password_check(password)
        print(f"请求生成: {request['session_id']}")
        
        # 模拟服务端处理
        response = client._simulate_server_response(request)
        
        # 处理结果
        is_compromised = client.process_server_response(response)
        status = "已泄露" if is_compromised else "安全"
        print(f"检查结果: {status}")
        
        # 密码强度分析
        strength = client.check_password_strength(password)
        print(f"密码强度: {strength['strength_level']} (评分: {strength['strength_score']}/100)")
        print(f"熵值估算: {strength['entropy_estimate']:.1f} bits")
    
    print(f"\n\n2. 批量密码检查演示")
    print("-" * 40)
    
    results = client.batch_check_passwords(test_passwords)
    
    print("\n批量检查结果:")
    for password, is_compromised in results.items():
        status = "已泄露" if is_compromised else "安全"
        strength = client.check_password_strength(password)
        print(f"'{password}': {status} (强度: {strength['strength_level']})")
    
    print(f"\n检查完成！共检查 {len(test_passwords)} 个密码")


if __name__ == "__main__":
    demo_client()
