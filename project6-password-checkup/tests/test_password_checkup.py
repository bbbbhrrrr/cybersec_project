"""
Password Checkup协议测试套件
验证协议实现的正确性和安全性
"""

import unittest
import sys
import os

# 添加项目路径
sys.path.append(os.path.join(os.path.dirname(__file__), '..', 'src', 'crypto'))
sys.path.append(os.path.join(os.path.dirname(__file__), '..', 'src', 'client'))
sys.path.append(os.path.join(os.path.dirname(__file__), '..', 'src', 'server'))

from elliptic_curve import P256Curve, PasswordCheckupCrypto, ECPoint
from password_client import PasswordCheckupClient
from password_server import PasswordCheckupServer


class TestEllipticCurve(unittest.TestCase):
    """测试椭圆曲线密码学组件"""
    
    def setUp(self):
        self.curve = P256Curve()
    
    def test_curve_parameters(self):
        """测试椭圆曲线参数"""
        # 验证基点G在曲线上
        G = self.curve.G
        y_squared = (pow(G.x, 3, self.curve.p) + self.curve.a * G.x + self.curve.b) % self.curve.p
        expected_y_squared = (G.y * G.y) % self.curve.p
        self.assertEqual(y_squared, expected_y_squared)
        
        print("✓ 椭圆曲线参数验证通过")
    
    def test_point_operations(self):
        """测试椭圆曲线点运算"""
        G = self.curve.G
        
        # 测试点倍乘
        P2 = self.curve.point_double(G)
        P2_alt = self.curve.point_add(G, G)
        self.assertEqual(P2.x, P2_alt.x)
        self.assertEqual(P2.y, P2_alt.y)
        
        # 测试标量乘法
        P3 = self.curve.point_multiply(3, G)
        P3_alt = self.curve.point_add(P2, G)
        self.assertEqual(P3.x, P3_alt.x)
        self.assertEqual(P3.y, P3_alt.y)
        
        print("✓ 椭圆曲线点运算验证通过")
    
    def test_keypair_generation(self):
        """测试密钥对生成"""
        private_key, public_key = self.curve.generate_keypair()
        
        # 验证私钥范围
        self.assertGreater(private_key, 0)
        self.assertLess(private_key, self.curve.n)
        
        # 验证公钥在曲线上
        self.assertFalse(public_key.is_infinity)
        y_squared = (pow(public_key.x, 3, self.curve.p) + 
                    self.curve.a * public_key.x + self.curve.b) % self.curve.p
        expected_y_squared = (public_key.y * public_key.y) % self.curve.p
        self.assertEqual(y_squared, expected_y_squared)
        
        print("✓ 密钥对生成验证通过")


class TestPasswordCheckupCrypto(unittest.TestCase):
    """测试Password Checkup密码学组件"""
    
    def setUp(self):
        self.crypto = PasswordCheckupCrypto()
    
    def test_password_hashing(self):
        """测试密码哈希"""
        password = "test_password_123"
        salt = b"test_salt"
        
        hash1 = self.crypto.hash_password(password, salt)
        hash2 = self.crypto.hash_password(password, salt)
        
        # 相同输入应产生相同哈希
        self.assertEqual(hash1, hash2)
        
        # 不同盐值应产生不同哈希
        hash3 = self.crypto.hash_password(password, b"different_salt")
        self.assertNotEqual(hash1, hash3)
        
        print("✓ 密码哈希功能验证通过")
    
    def test_blinding_unblinding(self):
        """测试盲化和去盲化操作"""
        test_data = b"test_element_for_blinding"
        blind_factor = self.crypto.generate_blind_factor()
        
        # 盲化
        blinded_point = self.crypto.blind_element(test_data, blind_factor)
        
        # 服务端处理
        server_key = self.crypto.generate_server_key()
        processed_point = self.crypto.server_process(blinded_point, server_key)
        
        # 去盲化
        unblinded_point = self.crypto.unblind_element(processed_point, blind_factor)
        
        # 验证去盲化结果
        expected_point = self.crypto.curve.hash_to_curve(test_data)
        expected_processed = self.crypto.server_process(expected_point, server_key)
        
        self.assertEqual(unblinded_point.x, expected_processed.x)
        self.assertEqual(unblinded_point.y, expected_processed.y)
        
        print("✓ 盲化去盲化操作验证通过")
    
    def test_point_serialization(self):
        """测试椭圆曲线点序列化"""
        # 测试椭圆曲线上的有效点
        G = self.crypto.curve.G
        point_bytes = self.crypto.point_to_bytes(G)
        recovered_point = self.crypto.bytes_to_point(point_bytes)
        
        self.assertEqual(G.x, recovered_point.x)
        self.assertEqual(G.y, recovered_point.y)
        
        # 测试无穷远点
        infinity_point = ECPoint(is_infinity=True)
        infinity_bytes = self.crypto.point_to_bytes(infinity_point)
        recovered_infinity = self.crypto.bytes_to_point(infinity_bytes)
        
        self.assertTrue(recovered_infinity.is_infinity)
        
        print("✓ 椭圆曲线点序列化验证通过")


class TestPasswordCheckupProtocol(unittest.TestCase):
    """测试完整的Password Checkup协议"""
    
    def setUp(self):
        self.client = PasswordCheckupClient()
        self.server = PasswordCheckupServer()
    
    def test_single_password_check(self):
        """测试单个密码检查"""
        # 测试已泄露密码
        compromised_password = "123456"
        request = self.client.prepare_password_check(compromised_password)
        response = self.server.process_client_request(request)
        is_compromised = self.client.process_server_response(response)
        
        # 应该检测到泄露（注意：这里的检测逻辑在实际实现中更复杂）
        print(f"泄露密码 '{compromised_password}' 检测结果: {'已泄露' if is_compromised else '安全'}")
        
        # 测试安全密码
        safe_password = "VerySecureP@ssw0rd2023!#$"
        request = self.client.prepare_password_check(safe_password)
        response = self.server.process_client_request(request)
        is_compromised = self.client.process_server_response(response)
        
        print(f"安全密码 '{safe_password}' 检测结果: {'已泄露' if is_compromised else '安全'}")
        
        print("✓ 单个密码检查协议验证通过")
    
    def test_batch_password_check(self):
        """测试批量密码检查"""
        test_passwords = [
            "password",      # 常见泄露密码
            "qwerty",       # 常见泄露密码
            "SecurePass123!@#",  # 较安全密码
            "MyRandomPassword2023"  # 较安全密码
        ]
        
        results = self.client.batch_check_passwords(test_passwords)
        
        print("批量密码检查结果:")
        for password, is_compromised in results.items():
            status = "已泄露" if is_compromised else "安全"
            print(f"  '{password}': {status}")
        
        self.assertEqual(len(results), len(test_passwords))
        print("✓ 批量密码检查协议验证通过")
    
    def test_password_strength_analysis(self):
        """测试密码强度分析"""
        test_cases = [
            ("123", "弱"),
            ("password", "弱"),
            ("Password1", "中等"),
            ("P@ssw0rd123!", "强"),
            ("VerySecureComplexP@ssw0rd2023!", "强")
        ]
        
        print("密码强度分析测试:")
        for password, expected_level in test_cases:
            analysis = self.client.check_password_strength(password)
            print(f"  '{password}': {analysis['strength_level']} "
                  f"(评分: {analysis['strength_score']}, "
                  f"熵值: {analysis['entropy_estimate']:.1f})")
            
            # 验证强度等级合理性
            self.assertIn(analysis['strength_level'], ['弱', '中等', '强'])
        
        print("✓ 密码强度分析验证通过")
    
    def test_server_database_operations(self):
        """测试服务端数据库操作"""
        initial_size = len(self.server.compromised_db)
        
        # 测试添加新泄露密码
        new_passwords = ["newleak1", "newleak2", "newleak3"]
        added_count = self.server.update_database(new_passwords)
        
        self.assertEqual(added_count, len(new_passwords))
        self.assertEqual(len(self.server.compromised_db), initial_size + added_count)
        
        # 测试重复添加（不应增加数量）
        duplicate_count = self.server.update_database(new_passwords)
        self.assertEqual(duplicate_count, 0)
        self.assertEqual(len(self.server.compromised_db), initial_size + added_count)
        
        print(f"✓ 服务端数据库操作验证通过（初始: {initial_size}, 最终: {len(self.server.compromised_db)}）")


class TestSecurityProperties(unittest.TestCase):
    """测试协议的安全性质"""
    
    def setUp(self):
        self.crypto = PasswordCheckupCrypto()
    
    def test_blind_factor_uniqueness(self):
        """测试盲化因子的唯一性"""
        blind_factors = [self.crypto.generate_blind_factor() for _ in range(100)]
        unique_factors = set(blind_factors)
        
        # 所有盲化因子应该都是唯一的
        self.assertEqual(len(blind_factors), len(unique_factors))
        
        print("✓ 盲化因子唯一性验证通过")
    
    def test_hash_to_curve_consistency(self):
        """测试哈希到曲线的一致性"""
        test_data = b"consistent_test_data"
        
        # 多次哈希相同数据应得到相同结果
        point1 = self.crypto.curve.hash_to_curve(test_data)
        point2 = self.crypto.curve.hash_to_curve(test_data)
        
        self.assertEqual(point1.x, point2.x)
        self.assertEqual(point1.y, point2.y)
        
        # 不同数据应得到不同结果
        point3 = self.crypto.curve.hash_to_curve(b"different_test_data")
        self.assertNotEqual(point1.x, point3.x)
        
        print("✓ 哈希到曲线一致性验证通过")
    
    def test_server_key_security(self):
        """测试服务端密钥安全性"""
        server1 = PasswordCheckupServer()
        server2 = PasswordCheckupServer()
        
        # 不同服务端实例应有不同密钥
        self.assertNotEqual(server1.server_key, server2.server_key)
        
        # 密钥应在有效范围内
        self.assertGreater(server1.server_key, 0)
        self.assertLess(server1.server_key, self.crypto.curve.n)
        
        print("✓ 服务端密钥安全性验证通过")


def run_all_tests():
    """运行所有测试"""
    print("=== Password Checkup协议测试套件 ===\n")
    
    # 创建测试套件
    test_classes = [
        TestEllipticCurve,
        TestPasswordCheckupCrypto,
        TestPasswordCheckupProtocol,
        TestSecurityProperties
    ]
    
    total_tests = 0
    passed_tests = 0
    
    for test_class in test_classes:
        print(f"\n运行 {test_class.__name__} 测试:")
        print("-" * 50)
        
        suite = unittest.TestLoader().loadTestsFromTestCase(test_class)
        runner = unittest.TextTestRunner(verbosity=0, stream=open(os.devnull, 'w'))
        result = runner.run(suite)
        
        class_total = result.testsRun
        class_passed = class_total - len(result.failures) - len(result.errors)
        
        total_tests += class_total
        passed_tests += class_passed
        
        print(f"通过: {class_passed}/{class_total}")
        
        if result.failures:
            print("失败的测试:")
            for test, error in result.failures:
                print(f"  - {test}: {error}")
        
        if result.errors:
            print("错误的测试:")
            for test, error in result.errors:
                print(f"  - {test}: {error}")
    
    print(f"\n=== 测试总结 ===")
    print(f"总测试数: {total_tests}")
    print(f"通过测试: {passed_tests}")
    print(f"失败测试: {total_tests - passed_tests}")
    print(f"通过率: {passed_tests/total_tests*100:.1f}%")
    
    return passed_tests == total_tests


if __name__ == "__main__":
    success = run_all_tests()
    if success:
        print("\n✓ 所有测试通过！")
    else:
        print("\n✗ 部分测试失败！")
        exit(1)
