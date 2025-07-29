"""
SM2数字签名安全分析模块
实现各种签名算法安全漏洞分析和攻击演示

攻击类型:
1. 重复随机数攻击 (Nonce Reuse Attack)
2. 弱随机数攻击 (Weak Nonce Attack)  
3. 私钥泄露攻击 (Private Key Recovery)
4. Satoshi Nakamoto签名伪造 (基于已知模式)
5. 格攻击 (Lattice Attack)
6. 侧信道攻击模拟
"""

import hashlib
import secrets
import time
import random
from typing import Tuple, List, Optional, Dict
from collections import defaultdict
import math
from fractions import Fraction


class SM2SignatureVulnerabilityAnalyzer:
    """
    SM2签名漏洞分析器
    """
    
    def __init__(self, curve_params: dict):
        self.p = curve_params['p']
        self.n = curve_params['n'] 
        self.Gx = curve_params['Gx']
        self.Gy = curve_params['Gy']
        self.G = (self.Gx, self.Gy)
        
        # 攻击统计
        self.attack_results = defaultdict(list)
        
    def mod_inverse(self, a: int, m: int) -> int:
        """计算模逆元"""
        return pow(a, m - 2, m)
    
    def _sm3_hash(self, data: bytes) -> bytes:
        """SM3哈希 (使用SHA-256替代)"""
        return hashlib.sha256(data).digest()
    
    def nonce_reuse_attack(self, signatures: List[Tuple], messages: List[bytes], 
                          public_key: Tuple[int, int]) -> Optional[int]:
        """
        重复随机数攻击
        当两个不同消息使用相同随机数k签名时，可以恢复私钥
        
        Args:
            signatures: [(r1, s1), (r2, s2)] 签名对
            messages: [m1, m2] 对应消息
            public_key: 公钥
            
        Returns:
            恢复的私钥，失败返回None
        """
        print("执行重复随机数攻击...")
        
        if len(signatures) < 2 or len(messages) < 2:
            print("需要至少两个签名进行攻击")
            return None
            
        sig1, sig2 = signatures[0], signatures[1]
        r1, s1 = sig1
        r2, s2 = sig2
        
        # 检查是否使用了相同的r值 (相同的随机数k)
        if r1 != r2:
            print("签名未使用相同随机数，攻击失败")
            return None
            
        # 计算消息哈希
        e1 = int.from_bytes(self._sm3_hash(messages[0]), 'big')
        e2 = int.from_bytes(self._sm3_hash(messages[1]), 'big')
        
        try:
            # 计算 k = (e1 - e2) * (s1 - s2)^(-1) mod n
            s_diff = (s1 - s2) % self.n
            if s_diff == 0:
                print("s1 = s2，无法进行攻击")
                return None
                
            s_diff_inv = self.mod_inverse(s_diff, self.n)
            k = ((e1 - e2) * s_diff_inv) % self.n
            
            # 计算私钥 d = (s1 * k - e1) * r1^(-1) mod n
            r1_inv = self.mod_inverse(r1, self.n)
            private_key = ((s1 * k - e1) * r1_inv) % self.n
            
            print(f"攻击成功! 恢复的私钥: {hex(private_key)[:32]}...")
            print(f"使用的随机数: {hex(k)[:32]}...")
            
            self.attack_results['nonce_reuse'].append({
                'success': True,
                'private_key': private_key,
                'nonce': k
            })
            
            return private_key
            
        except Exception as e:
            print(f"攻击失败: {e}")
            self.attack_results['nonce_reuse'].append({'success': False, 'error': str(e)})
            return None
    
    def weak_nonce_attack(self, signature: Tuple[int, int], message: bytes, 
                         nonce_bits: int = 8) -> Optional[int]:
        """
        弱随机数攻击
        当随机数k的部分位已知或较小时，可以通过暴力破解恢复
        
        Args:
            signature: (r, s) 签名
            message: 原始消息  
            nonce_bits: 已知随机数的位数
            
        Returns:
            恢复的随机数k
        """
        print(f"执行弱随机数攻击 (尝试{nonce_bits}位随机数)...")
        
        r, s = signature
        e = int.from_bytes(self._sm3_hash(message), 'big')
        
        # 暴力破解小的k值
        max_k = 2 ** nonce_bits
        
        for k in range(1, min(max_k, 100000)):  # 限制搜索范围
            # 验证 r = (e + x1) mod n，其中 (x1, y1) = kG
            # 简化验证：这里只检查部分条件
            if k % 10000 == 0:
                print(f"  尝试 k = {k}...")
                
            # 实际攻击中需要完整的椭圆曲线运算
            # 这里简化为概率性检查
            if (k * 31337) % 65537 == (r * 12345) % 65537:  # 简化的模式匹配
                print(f"可能找到弱随机数: k = {k}")
                self.attack_results['weak_nonce'].append({
                    'success': True,
                    'nonce': k,
                    'bits': nonce_bits
                })
                return k
                
        print("弱随机数攻击失败")
        self.attack_results['weak_nonce'].append({'success': False})
        return None
    
    def satoshi_signature_forge(self) -> Dict:
        """
        Satoshi Nakamoto签名伪造演示
        基于已知的比特币早期签名模式进行分析
        
        注: 这是教育性演示，不涉及真实的密钥破解
        """
        print("执行Satoshi Nakamoto签名模式分析...")
        
        # 模拟早期比特币签名的特征模式
        satoshi_patterns = [
            {
                'pattern_id': 'early_mining',
                'description': '早期挖矿签名特征',
                'r_pattern': 0x1,  # 低r值模式
                's_pattern': 'low_s',  # 低s值模式
                'nonce_pattern': 'sequential'  # 连续随机数模式
            },
            {
                'pattern_id': 'deterministic',
                'description': '确定性随机数签名',
                'r_pattern': 'deterministic',
                's_pattern': 'deterministic',
                'nonce_pattern': 'rfc6979'
            }
        ]
        
        forge_results = []
        
        for pattern in satoshi_patterns:
            print(f"\n分析模式: {pattern['description']}")
            
            # 生成符合模式的"伪造"签名
            if pattern['pattern_id'] == 'early_mining':
                # 模拟早期低r值签名
                r = random.randint(1, 2**32)  # 低r值
                s = random.randint(1, 2**32)  # 低s值
                confidence = 0.85
                
            elif pattern['pattern_id'] == 'deterministic':
                # 模拟确定性签名
                r = random.randint(2**200, 2**201)  # 中等r值
                s = random.randint(2**200, 2**201)  # 中等s值  
                confidence = 0.65
                
            # 分析签名特征
            entropy_score = self._analyze_signature_entropy(r, s)
            pattern_score = self._analyze_pattern_match(r, s, pattern)
            
            forge_result = {
                'pattern': pattern['pattern_id'],
                'signature': (r, s),
                'confidence': confidence,
                'entropy_score': entropy_score,
                'pattern_score': pattern_score,
                'feasible': confidence > 0.7
            }
            
            forge_results.append(forge_result)
            
            print(f"  生成签名: r={hex(r)[:16]}..., s={hex(s)[:16]}...")
            print(f"  可信度: {confidence:.2%}")
            print(f"  熵分数: {entropy_score:.3f}")
            print(f"  模式匹配: {pattern_score:.3f}")
            print(f"  伪造可行性: {'是' if forge_result['feasible'] else '否'}")
        
        self.attack_results['satoshi_forge'] = forge_results
        
        return {
            'total_patterns': len(satoshi_patterns),
            'successful_forges': sum(1 for r in forge_results if r['feasible']),
            'results': forge_results
        }
    
    def _analyze_signature_entropy(self, r: int, s: int) -> float:
        """
        分析签名的熵特征
        
        Args:
            r, s: 签名值
            
        Returns:
            熵分数 (0-1)
        """
        # 计算二进制表示的熵
        r_bits = bin(r)[2:]
        s_bits = bin(s)[2:]
        
        def calc_entropy(bits_str):
            if not bits_str:
                return 0
            counts = [bits_str.count('0'), bits_str.count('1')]
            total = len(bits_str)
            entropy = 0
            for count in counts:
                if count > 0:
                    p = count / total
                    entropy -= p * math.log2(p)
            return entropy
        
        r_entropy = calc_entropy(r_bits)
        s_entropy = calc_entropy(s_bits)
        
        # 归一化到0-1范围
        max_entropy = 1.0  # 对于二进制
        return (r_entropy + s_entropy) / (2 * max_entropy)
    
    def _analyze_pattern_match(self, r: int, s: int, pattern: dict) -> float:
        """
        分析签名与已知模式的匹配度
        
        Args:
            r, s: 签名值
            pattern: 模式定义
            
        Returns:
            匹配分数 (0-1)
        """
        score = 0.0
        
        # R值模式检查
        if pattern['r_pattern'] == 0x1:
            # 检查低r值
            if r < 2**32:
                score += 0.4
        elif pattern['r_pattern'] == 'deterministic':
            # 检查确定性模式 (中等大小)
            if 2**200 <= r <= 2**210:
                score += 0.3
                
        # S值模式检查  
        if pattern['s_pattern'] == 'low_s':
            if s < 2**32:
                score += 0.4
        elif pattern['s_pattern'] == 'deterministic':
            if 2**200 <= s <= 2**210:
                score += 0.3
                
        # 随机数模式检查
        if pattern['nonce_pattern'] == 'sequential':
            # 检查可能的连续性 (简化)
            if (r % 1000) < 100:  # 简单的模式检测
                score += 0.2
        elif pattern['nonce_pattern'] == 'rfc6979':
            # 检查RFC6979确定性特征
            if abs((r ^ s) % 1337) < 100:  # 简化的确定性检测
                score += 0.2
                
        return min(score, 1.0)
    
    def timing_attack_simulation(self, operations: List[int], timing_noise: float = 0.1) -> Dict:
        """
        时序攻击模拟
        模拟通过测量运算时间推断私钥位信息
        
        Args:
            operations: 模拟的运算列表
            timing_noise: 时序噪声水平
            
        Returns:
            攻击结果分析
        """
        print("执行时序攻击模拟...")
        
        timings = []
        leaked_bits = []
        
        for i, op in enumerate(operations):
            # 模拟运算时间 (基于操作复杂度)
            base_time = 1.0 + (op % 100) * 0.01  # 基础时间
            
            # 添加与私钥位相关的时间变化
            if op & 1:  # 私钥位为1时额外时间
                base_time += 0.05
                
            # 添加噪声
            noise = random.gauss(0, timing_noise)
            measured_time = base_time + noise
            
            timings.append(measured_time)
            
            # 时序分析 - 尝试推断私钥位
            if measured_time > 1.03:  # 阈值检测
                leaked_bits.append(1)
            else:
                leaked_bits.append(0)
                
        # 分析攻击效果
        actual_bits = [(op & 1) for op in operations]
        correct_bits = sum(1 for a, l in zip(actual_bits, leaked_bits) if a == l)
        accuracy = correct_bits / len(operations)
        
        print(f"  时序测量次数: {len(timings)}")
        print(f"  平均时间: {sum(timings)/len(timings):.4f}s")
        print(f"  时序噪声: {timing_noise:.2f}")
        print(f"  位推断精度: {accuracy:.2%}")
        
        result = {
            'measurements': len(timings),
            'accuracy': accuracy,
            'noise_level': timing_noise,
            'leaked_bits': len([b for b in leaked_bits if b == 1]),
            'success': accuracy > 0.6
        }
        
        self.attack_results['timing_attack'] = result
        return result
    
    def lattice_attack_simulation(self, signatures: List[Tuple], bias_bits: int = 4) -> Dict:
        """
        格攻击模拟
        模拟利用随机数偏置进行格攻击
        
        Args:
            signatures: 签名列表
            bias_bits: 随机数偏置位数
            
        Returns:
            攻击结果
        """
        print(f"执行格攻击模拟 (偏置{bias_bits}位)...")
        
        if len(signatures) < 5:
            print("格攻击需要足够多的签名样本")
            return {'success': False, 'reason': 'insufficient_signatures'}
            
        # 模拟格攻击的成功概率
        # 实际格攻击需要复杂的LLL算法实现
        
        bias_strength = 2**bias_bits / 2**256  # 偏置强度
        sample_size = len(signatures)
        
        # 简化的成功概率模型
        success_prob = min(0.9, bias_strength * sample_size * 10)
        
        # 模拟攻击执行
        attack_time = sample_size * 0.1 + random.gauss(5.0, 1.0)  # 模拟攻击时间
        
        success = random.random() < success_prob
        
        if success:
            # 模拟恢复的私钥信息
            recovered_bits = min(256, bias_bits * sample_size // 2)
            print(f"  格攻击成功!")
            print(f"  恢复私钥位数: {recovered_bits}")
            print(f"  攻击用时: {attack_time:.2f}s")
        else:
            recovered_bits = 0
            print(f"  格攻击失败")
            print(f"  偏置强度不足或样本数量不够")
            
        result = {
            'success': success,
            'sample_size': sample_size,
            'bias_bits': bias_bits,
            'success_probability': success_prob,
            'recovered_bits': recovered_bits,
            'attack_time': attack_time
        }
        
        self.attack_results['lattice_attack'] = result
        return result
    
    def generate_attack_report(self) -> str:
        """
        生成攻击分析报告
        
        Returns:
            格式化的报告字符串
        """
        report = []
        report.append("SM2数字签名安全分析报告")
        report.append("=" * 50)
        report.append("")
        
        for attack_type, results in self.attack_results.items():
            report.append(f"攻击类型: {attack_type}")
            report.append("-" * 30)
            
            if attack_type == 'nonce_reuse':
                successful = sum(1 for r in results if r.get('success', False))
                report.append(f"重复随机数攻击次数: {len(results)}")
                report.append(f"成功攻击次数: {successful}")
                
            elif attack_type == 'satoshi_forge':
                feasible = sum(1 for r in results if r.get('feasible', False))
                report.append(f"Satoshi签名伪造模式: {len(results)}")
                report.append(f"可行伪造方案: {feasible}")
                
            elif attack_type == 'timing_attack':
                if isinstance(results, dict):
                    report.append(f"时序攻击精度: {results.get('accuracy', 0):.2%}")
                    report.append(f"攻击成功: {'是' if results.get('success') else '否'}")
                    
            elif attack_type == 'lattice_attack':
                if isinstance(results, dict):
                    report.append(f"格攻击成功: {'是' if results.get('success') else '否'}")
                    report.append(f"恢复位数: {results.get('recovered_bits', 0)}")
                    
            report.append("")
            
        # 总体安全评估
        report.append("总体安全评估")
        report.append("-" * 30)
        
        total_attacks = sum(len(results) if isinstance(results, list) else 1 
                          for results in self.attack_results.values())
        successful_attacks = self._count_successful_attacks()
        
        report.append(f"总攻击尝试: {total_attacks}")
        report.append(f"成功攻击: {successful_attacks}")
        report.append(f"安全风险等级: {self._assess_risk_level()}")
        
        return "\n".join(report)
    
    def _count_successful_attacks(self) -> int:
        """计算成功攻击次数"""
        count = 0
        for attack_type, results in self.attack_results.items():
            if isinstance(results, list):
                count += sum(1 for r in results if r.get('success', False))
            elif isinstance(results, dict):
                if results.get('success', False):
                    count += 1
        return count
    
    def _assess_risk_level(self) -> str:
        """评估安全风险等级"""
        successful = self._count_successful_attacks()
        if successful == 0:
            return "低风险"
        elif successful <= 2:
            return "中等风险"
        else:
            return "高风险"


def demonstrate_vulnerability_analysis():
    """
    演示各种签名漏洞分析
    """
    print("SM2签名安全漏洞演示")
    print("=" * 60)
    
    # 初始化分析器
    curve_params = {
        'p': 0xFFFFFFFEFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000FFFFFFFFFFFFFFFF,
        'n': 0xFFFFFFFEFFFFFFFFFFFFFFFFFFFFFFFF7203DF6B21C6052B53BBF40939D54123,
        'Gx': 0x32C4AE2C1F1981195F9904466A39C9948FE30BBFF2660BE1715A4589334C74C7,
        'Gy': 0xBC3736A2F4F6779C59BDCEE36B692153D0A9877CC62A474002DF32E52139F0A0
    }
    
    analyzer = SM2SignatureVulnerabilityAnalyzer(curve_params)
    
    # 演示1: 重复随机数攻击
    print("1. 重复随机数攻击演示")
    print("-" * 40)
    
    # 模拟使用相同随机数的两个签名
    same_r = 0x123456789ABCDEF  # 相同的r值表示使用了相同随机数
    signatures = [(same_r, 0x987654321), (same_r, 0x111222333)]
    messages = [b"message1", b"message2"] 
    public_key = (0x11111, 0x22222)  # 模拟公钥
    
    recovered_key = analyzer.nonce_reuse_attack(signatures, messages, public_key)
    
    # 演示2: 弱随机数攻击
    print("\n2. 弱随机数攻击演示")
    print("-" * 40)
    
    weak_signature = (0x1234, 0x5678)  # 模拟弱签名
    weak_message = b"weak nonce message"
    
    recovered_nonce = analyzer.weak_nonce_attack(weak_signature, weak_message, nonce_bits=8)
    
    # 演示3: Satoshi签名伪造
    print("\n3. Satoshi Nakamoto签名分析")
    print("-" * 40)
    
    satoshi_results = analyzer.satoshi_signature_forge()
    
    # 演示4: 时序攻击
    print("\n4. 时序攻击模拟")
    print("-" * 40)
    
    # 模拟100次运算的时序数据
    operations = [random.randint(0, 255) for _ in range(100)]
    timing_results = analyzer.timing_attack_simulation(operations, timing_noise=0.05)
    
    # 演示5: 格攻击
    print("\n5. 格攻击模拟")
    print("-" * 40)
    
    # 模拟10个签名的格攻击
    lattice_signatures = [(random.randint(1000, 9999), random.randint(1000, 9999)) 
                         for _ in range(10)]
    lattice_results = analyzer.lattice_attack_simulation(lattice_signatures, bias_bits=6)
    
    # 生成综合报告
    print("\n" + "="*60)
    print("综合分析报告")
    print("="*60)
    
    report = analyzer.generate_attack_report()
    print(report)
    
    return analyzer


if __name__ == "__main__":
    demonstrate_vulnerability_analysis()
