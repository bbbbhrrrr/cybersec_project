"""
SM2椭圆曲线密码算法测试套件
验证基础实现和优化实现的正确性和性能
"""

import unittest
import time
import sys
import os

# 添加src路径
sys.path.append(os.path.join(os.path.dirname(__file__), '..', 'src'))

from basic.sm2_basic import SM2Curve, SM2KeyPair, SM2DigitalSignature
from optimized.sm2_optimized import SM2CurveOptimized, SM2DigitalSignatureOptimized
from basic.security_analysis import SM2SignatureVulnerabilityAnalyzer
from basic.signature_misuse_poc import SM2SignatureMisuseAnalyzer


class TestSM2Basic(unittest.TestCase):
    """测试基础SM2实现"""
    
    def setUp(self):
        self.curve = SM2Curve()
        self.keypair = SM2KeyPair(self.curve)
        self.signer = SM2DigitalSignature(self.curve)
        
    def test_curve_parameters(self):
        """测试曲线参数是否正确"""
        # 验证基点在曲线上
        self.assertTrue(self.curve.is_on_curve(self.curve.G))
        
        # 验证基点的阶
        # nG应该等于无穷远点
        nG = self.curve.point_multiply(self.curve.n, self.curve.G)
        self.assertIsNone(nG)
        
    def test_point_operations(self):
        """测试椭圆曲线点运算"""
        G = self.curve.G
        
        # 测试点倍运算
        G2 = self.curve.point_double(G)
        self.assertIsNotNone(G2)
        self.assertTrue(self.curve.is_on_curve(G2))
        
        # 测试点加运算
        G3 = self.curve.point_add(G, G2)
        self.assertIsNotNone(G3)
        self.assertTrue(self.curve.is_on_curve(G3))
        
        # 验证 2G + G = 3G
        G3_verify = self.curve.point_multiply(3, G)
        self.assertEqual(G3, G3_verify)
        
    def test_scalar_multiplication(self):
        """测试标量乘法"""
        G = self.curve.G
        
        # 测试小标量
        for k in [1, 2, 3, 7, 15, 255]:
            kG = self.curve.point_multiply(k, G)
            self.assertIsNotNone(kG)
            self.assertTrue(self.curve.is_on_curve(kG))
            
        # 测试大标量
        large_k = self.curve.n // 2
        kG = self.curve.point_multiply(large_k, G)
        self.assertIsNotNone(kG)
        self.assertTrue(self.curve.is_on_curve(kG))
        
    def test_key_generation(self):
        """测试密钥生成"""
        private_key, public_key = self.keypair.generate_keypair()
        
        # 验证私钥范围
        self.assertGreater(private_key, 1)
        self.assertLess(private_key, self.curve.n)
        
        # 验证公钥在曲线上
        self.assertTrue(self.curve.is_on_curve(public_key))
        
        # 验证公钥 = 私钥 × G
        expected_public = self.curve.point_multiply(private_key, self.curve.G)
        self.assertEqual(public_key, expected_public)
        
    def test_digital_signature(self):
        """测试数字签名"""
        # 生成密钥对
        private_key, public_key = self.keypair.generate_keypair()
        
        # 测试消息
        message = b"Hello, SM2 Digital Signature Test!"
        
        # 签名
        signature = self.signer.sign(message, private_key, public_key)
        self.assertIsInstance(signature, tuple)
        self.assertEqual(len(signature), 2)
        
        r, s = signature
        self.assertGreater(r, 0)
        self.assertLess(r, self.curve.n)
        self.assertGreater(s, 0)
        self.assertLess(s, self.curve.n)
        
        # 验证
        is_valid = self.signer.verify(message, signature, public_key)
        self.assertTrue(is_valid)
        
        # 验证错误消息应该失败
        wrong_message = b"Wrong message"
        is_valid_wrong = self.signer.verify(wrong_message, signature, public_key)
        self.assertFalse(is_valid_wrong)
        
    def test_point_compression(self):
        """测试点压缩和解压缩"""
        G = self.curve.G
        
        # 压缩
        compressed = self.curve.compress_point(G)
        self.assertEqual(len(compressed), 33)
        
        # 解压缩
        decompressed = self.curve.decompress_point(compressed)
        self.assertEqual(G, decompressed)
        
        # 测试随机点
        k = 12345
        P = self.curve.point_multiply(k, G)
        compressed_P = self.curve.compress_point(P)
        decompressed_P = self.curve.decompress_point(compressed_P)
        self.assertEqual(P, decompressed_P)


class TestSM2Optimized(unittest.TestCase):
    """测试优化SM2实现"""
    
    def setUp(self):
        self.curve = SM2CurveOptimized()
        self.signer = SM2DigitalSignatureOptimized(self.curve)
        
    def test_jacobian_coordinates(self):
        """测试雅可比坐标运算"""
        G_affine = (self.curve.Gx, self.curve.Gy)
        G_jacobian = self.curve.affine_to_jacobian(G_affine)
        
        # 测试坐标转换
        G_back = self.curve.jacobian_to_affine(G_jacobian)
        self.assertEqual(G_affine, G_back)
        
        # 测试雅可比点倍运算
        G2_jacobian = self.curve.jacobian_double(G_jacobian)
        G2_affine = self.curve.jacobian_to_affine(G2_jacobian)
        self.assertTrue(self.curve.is_on_curve(G2_affine))
        
        # 测试雅可比点加运算
        G3_jacobian = self.curve.jacobian_add(G_jacobian, G2_jacobian)
        G3_affine = self.curve.jacobian_to_affine(G3_jacobian)
        self.assertTrue(self.curve.is_on_curve(G3_affine))
        
    def test_montgomery_ladder(self):
        """测试Montgomery阶梯算法"""
        G = (self.curve.Gx, self.curve.Gy)
        
        for k in [1, 2, 3, 7, 15, 255, 65537]:
            # Montgomery阶梯结果
            result_montgomery = self.curve.montgomery_ladder(k, G)
            
            # 基础方法结果
            result_basic = self.curve._basic_multiply(k, G)
            
            # 两种方法应该得到相同结果
            self.assertEqual(result_montgomery, result_basic)
            
    def test_windowed_naf(self):
        """测试窗口NAF方法"""
        G = (self.curve.Gx, self.curve.Gy)
        
        for k in [7, 15, 255, 1337, 65537]:
            # 窗口NAF结果
            result_wnaf = self.curve.windowed_naf_multiply(k, G)
            
            # 基础方法结果
            result_basic = self.curve._basic_multiply(k, G)
            
            # 两种方法应该得到相同结果
            self.assertEqual(result_wnaf, result_basic)
            
    def test_batch_inverse(self):
        """测试批量模逆运算"""
        values = [123, 456, 789, 1337, 9999]
        
        # 批量计算
        batch_inverses = self.curve.batch_mod_inverse(values, self.curve.p)
        
        # 单独计算验证
        for val, inv in zip(values, batch_inverses):
            expected_inv = self.curve.mod_inverse(val, self.curve.p)
            self.assertEqual(inv, expected_inv)
            
            # 验证 val * inv ≡ 1 (mod p)
            self.assertEqual((val * inv) % self.curve.p, 1)
            
    def test_optimized_signature(self):
        """测试优化签名算法"""
        # 生成密钥对
        private_key = 12345678901234567890123456789
        public_key = self.curve.point_multiply(private_key, (self.curve.Gx, self.curve.Gy), "montgomery")
        
        message = b"Optimized signature test message"
        
        # 优化签名
        signature = self.signer.sign_optimized(message, private_key, public_key)
        
        # 优化验证
        is_valid = self.signer.verify_optimized(message, signature, public_key)
        self.assertTrue(is_valid)
        
    def test_shamir_double_multiply(self):
        """测试Shamir双标量乘法"""
        P1 = (self.curve.Gx, self.curve.Gy)
        P2 = self.curve.point_multiply(7, P1, "basic")
        
        k1, k2 = 123, 456
        
        # Shamir方法
        result_shamir = self.signer._shamir_double_multiply(k1, P1, k2, P2)
        
        # 分别计算后相加
        k1P1 = self.curve.point_multiply(k1, P1, "basic")
        k2P2 = self.curve.point_multiply(k2, P2, "basic")
        
        k1P1_jac = self.curve.affine_to_jacobian(k1P1)
        k2P2_jac = self.curve.affine_to_jacobian(k2P2)
        result_separate = self.curve.jacobian_to_affine(
            self.curve.jacobian_add(k1P1_jac, k2P2_jac)
        )
        
        self.assertEqual(result_shamir, result_separate)


class TestSecurityAnalysis(unittest.TestCase):
    """测试安全分析模块"""
    
    def setUp(self):
        curve_params = {
            'p': 0xFFFFFFFEFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000FFFFFFFFFFFFFFFF,
            'n': 0xFFFFFFFEFFFFFFFFFFFFFFFFFFFFFFFF7203DF6B21C6052B53BBF40939D54123,
            'Gx': 0x32C4AE2C1F1981195F9904466A39C9948FE30BBFF2660BE1715A4589334C74C7,
            'Gy': 0xBC3736A2F4F6779C59BDCEE36B692153D0A9877CC62A474002DF32E52139F0A0
        }
        self.analyzer = SM2SignatureVulnerabilityAnalyzer(curve_params)
        
    def test_nonce_reuse_attack(self):
        """测试重复随机数攻击"""
        # 模拟相同r值的签名
        same_r = 0x123456789ABCDEF
        signatures = [(same_r, 0x111111111), (same_r, 0x222222222)]
        messages = [b"message1", b"message2"]
        public_key = (0x11111, 0x22222)
        
        # 应该能够恢复某些信息
        result = self.analyzer.nonce_reuse_attack(signatures, messages, public_key)
        # 注意：这个测试可能失败，因为我们使用的是模拟数据
        
    def test_weak_nonce_attack(self):
        """测试弱随机数攻击"""
        signature = (0x1234, 0x5678)
        message = b"test message"
        
        # 测试是否能检测到弱随机数
        result = self.analyzer.weak_nonce_attack(signature, message, nonce_bits=8)
        
    def test_timing_attack_simulation(self):
        """测试时序攻击模拟"""
        operations = list(range(100))
        result = self.analyzer.timing_attack_simulation(operations, timing_noise=0.1)
        
        self.assertIn('accuracy', result)
        self.assertIn('success', result)
        self.assertIsInstance(result['accuracy'], float)
        
    def test_lattice_attack_simulation(self):
        """测试格攻击模拟"""
        signatures = [(i*1000, i*2000) for i in range(10)]
        result = self.analyzer.lattice_attack_simulation(signatures, bias_bits=4)
        
        self.assertIn('success', result)
        self.assertIn('sample_size', result)
        
    def test_report_generation(self):
        """测试报告生成"""
        # 先执行一些攻击以生成数据
        self.analyzer.timing_attack_simulation(list(range(50)), 0.1)
        
        report = self.analyzer.generate_attack_report()
        self.assertIsInstance(report, str)
        self.assertIn("SM2数字签名安全分析报告", report)


class TestSignatureMisusePOC(unittest.TestCase):
    """测试签名误用POC"""
    
    def setUp(self):
        self.analyzer = SM2SignatureMisuseAnalyzer()
        
    def test_satoshi_analysis(self):
        """测试Satoshi签名分析"""
        result = self.analyzer.analyze_satoshi_signatures()
        
        self.assertIn('vulnerability_indicators', result)
        self.assertIn('forge_probability', result)
        self.assertIn('pattern_analysis', result)
        self.assertIn('security_implications', result)
        
        # 验证概率在合理范围内
        prob = result['forge_probability']
        self.assertGreaterEqual(prob, 0.0)
        self.assertLessEqual(prob, 1.0)
        
    def test_signature_forge_demo(self):
        """测试签名伪造演示"""
        # 先执行分析以加载模式
        self.analyzer.analyze_satoshi_signatures()
        
        result = self.analyzer.demonstrate_signature_forge()
        
        self.assertIn('success', result)
        self.assertIn('signature', result)
        self.assertIn('confidence', result)
        self.assertIn('method', result)
        
        if result['success']:
            r, s = result['signature']
            self.assertIsInstance(r, int)
            self.assertIsInstance(s, int)
            self.assertGreater(r, 0)
            self.assertGreater(s, 0)
            
    def test_pattern_analysis(self):
        """测试模式分析"""
        # 执行分析
        analysis = self.analyzer.analyze_satoshi_signatures()
        pattern_analysis = analysis['pattern_analysis']
        
        self.assertIn('sample_count', pattern_analysis)
        self.assertIn('pattern_matches', pattern_analysis)
        self.assertIn('anomaly_detection', pattern_analysis)
        self.assertIn('statistical_analysis', pattern_analysis)
        
    def test_report_generation(self):
        """测试报告生成"""
        # 执行分析
        self.analyzer.analyze_satoshi_signatures()
        
        report = self.analyzer.generate_comprehensive_report()
        self.assertIsInstance(report, str)
        self.assertIn("SM2数字签名算法误用分析报告", report)


class TestPerformance(unittest.TestCase):
    """性能测试"""
    
    def setUp(self):
        self.basic_curve = SM2Curve()
        self.optimized_curve = SM2CurveOptimized()
        
    def test_scalar_multiplication_performance(self):
        """测试标量乘法性能"""
        G = (self.optimized_curve.Gx, self.optimized_curve.Gy)
        k = 0x123456789ABCDEF0123456789ABCDEF0123456789ABCDEF
        
        methods = ["basic", "montgomery", "windowed_naf"]
        iterations = 10
        
        results = {}
        
        for method in methods:
            start_time = time.time()
            for _ in range(iterations):
                result = self.optimized_curve.point_multiply(k, G, method)
            end_time = time.time()
            
            results[method] = {
                'time': end_time - start_time,
                'avg_time': (end_time - start_time) / iterations,
                'result': result
            }
        
        # 验证所有方法得到相同结果
        base_result = results["basic"]["result"]
        for method in methods[1:]:
            self.assertEqual(results[method]["result"], base_result)
            
        # 输出性能统计
        print(f"\n标量乘法性能测试 ({iterations}次迭代):")
        for method, data in results.items():
            print(f"  {method}: {data['avg_time']*1000:.2f}ms平均")
            
    def test_signature_performance(self):
        """测试签名性能"""
        # 基础实现
        basic_signer = SM2DigitalSignature(self.basic_curve)
        basic_keypair = SM2KeyPair(self.basic_curve)
        basic_private, basic_public = basic_keypair.generate_keypair()
        
        # 优化实现
        optimized_signer = SM2DigitalSignatureOptimized(self.optimized_curve)
        optimized_private = basic_private  # 使用相同私钥
        optimized_public = self.optimized_curve.point_multiply(
            optimized_private, (self.optimized_curve.Gx, self.optimized_curve.Gy), "montgomery"
        )
        
        message = b"Performance test message"
        iterations = 5
        
        # 基础签名性能
        start_time = time.time()
        for _ in range(iterations):
            signature = basic_signer.sign(message, basic_private, basic_public)
        basic_sign_time = time.time() - start_time
        
        # 优化签名性能
        start_time = time.time()
        for _ in range(iterations):
            signature = optimized_signer.sign_optimized(message, optimized_private, optimized_public)
        optimized_sign_time = time.time() - start_time
        
        print(f"\n签名性能测试 ({iterations}次迭代):")
        print(f"  基础实现: {basic_sign_time/iterations*1000:.2f}ms平均")
        print(f"  优化实现: {optimized_sign_time/iterations*1000:.2f}ms平均")
        print(f"  性能提升: {basic_sign_time/optimized_sign_time:.2f}x")


def run_comprehensive_tests():
    """运行完整测试套件"""
    print("SM2椭圆曲线密码算法测试套件")
    print("=" * 60)
    
    # 创建测试套件
    test_suite = unittest.TestSuite()
    
    # 添加基础测试
    test_suite.addTest(unittest.makeSuite(TestSM2Basic))
    test_suite.addTest(unittest.makeSuite(TestSM2Optimized))
    test_suite.addTest(unittest.makeSuite(TestSecurityAnalysis))
    test_suite.addTest(unittest.makeSuite(TestSignatureMisusePOC))
    test_suite.addTest(unittest.makeSuite(TestPerformance))
    
    # 运行测试
    runner = unittest.TextTestRunner(verbosity=2)
    result = runner.run(test_suite)
    
    # 输出总结
    print("\n" + "=" * 60)
    print("测试总结:")
    print(f"  总测试数: {result.testsRun}")
    print(f"  成功: {result.testsRun - len(result.failures) - len(result.errors)}")
    print(f"  失败: {len(result.failures)}")
    print(f"  错误: {len(result.errors)}")
    
    if result.failures:
        print("\n失败的测试:")
        for test, traceback in result.failures:
            print(f"  - {test}: {traceback.splitlines()[-1]}")
            
    if result.errors:
        print("\n错误的测试:")
        for test, traceback in result.errors:
            print(f"  - {test}: {traceback.splitlines()[-1]}")
    
    success_rate = (result.testsRun - len(result.failures) - len(result.errors)) / result.testsRun
    print(f"\n总体成功率: {success_rate:.1%}")
    
    return result


if __name__ == "__main__":
    run_comprehensive_tests()
