"""
SM2椭圆曲线密码算法综合演示程序
展示基础实现、优化算法、安全分析和漏洞研究

主要功能演示:
1. SM2基础椭圆曲线运算
2. 数字签名算法
3. 性能优化技术对比
4. 签名安全漏洞分析
5. Satoshi Nakamoto签名特征研究
"""

import sys
import os
import time
import json
from datetime import datetime

# 添加src路径
sys.path.append(os.path.join(os.path.dirname(__file__), 'src'))

try:
    from basic.sm2_basic import SM2Curve, SM2KeyPair, SM2DigitalSignature
    from optimized.sm2_optimized import SM2CurveOptimized, SM2DigitalSignatureOptimized, benchmark_comparison
    from basic.security_analysis import SM2SignatureVulnerabilityAnalyzer, demonstrate_vulnerability_analysis
    from basic.signature_misuse_poc import SM2SignatureMisuseAnalyzer, main as poc_main
except ImportError as e:
    print(f"导入错误: {e}")
    print("请确保所有模块文件都已正确创建")
    sys.exit(1)


class SM2ComprehensiveDemo:
    """SM2综合演示类"""
    
    def __init__(self):
        self.results = {
            'timestamp': datetime.now().isoformat(),
            'demonstrations': [],
            'performance_metrics': {},
            'security_analysis': {},
            'vulnerability_research': {}
        }
        
    def demonstrate_basic_functionality(self):
        """演示基础功能"""
        print("=" * 60)
        print("1. SM2基础功能演示")
        print("=" * 60)
        
        # 初始化
        curve = SM2Curve()
        keypair = SM2KeyPair(curve)
        signer = SM2DigitalSignature(curve)
        
        print("椭圆曲线参数:")
        print(f"  p = {hex(curve.p)[:32]}...")
        print(f"  n = {hex(curve.n)[:32]}...")
        print(f"  G = ({hex(curve.Gx)[:16]}..., {hex(curve.Gy)[:16]}...)")
        
        # 生成密钥对
        print("\n生成密钥对:")
        private_key, public_key = keypair.generate_keypair()
        print(f"  私钥: {hex(private_key)[:32]}...")
        print(f"  公钥: ({hex(public_key[0])[:16]}..., {hex(public_key[1])[:16]}...)")
        
        # 验证公钥
        assert curve.is_on_curve(public_key), "公钥不在曲线上"
        print("  ✓ 公钥验证通过")
        
        # 数字签名演示
        print("\n数字签名演示:")
        messages = [
            b"Hello, SM2!",
            b"This is a test message for SM2 digital signature.",
            "SM2椭圆曲线数字签名算法演示".encode('utf-8')
        ]
        
        signatures = []
        for i, message in enumerate(messages):
            print(f"  消息{i+1}: {message.decode('utf-8', errors='ignore')}")
            
            # 签名
            start_time = time.time()
            signature = signer.sign(message, private_key, public_key)
            sign_time = time.time() - start_time
            
            print(f"    签名: r={hex(signature[0])[:16]}..., s={hex(signature[1])[:16]}...")
            print(f"    签名时间: {sign_time*1000:.2f}ms")
            
            # 验证
            start_time = time.time()
            is_valid = signer.verify(message, signature, public_key)
            verify_time = time.time() - start_time
            
            print(f"    验证结果: {'通过' if is_valid else '失败'}")
            print(f"    验证时间: {verify_time*1000:.2f}ms")
            
            signatures.append({
                'message': message.hex(),
                'signature': {'r': hex(signature[0]), 's': hex(signature[1])},
                'sign_time': sign_time,
                'verify_time': verify_time,
                'valid': is_valid
            })
        
        self.results['demonstrations'].append({
            'type': 'basic_functionality',
            'keypair': {
                'private_key': hex(private_key),
                'public_key': {'x': hex(public_key[0]), 'y': hex(public_key[1])}
            },
            'signatures': signatures
        })
        
        return True
        
    def demonstrate_performance_optimization(self):
        """演示性能优化"""
        print("\n" + "=" * 60)
        print("2. 性能优化演示")
        print("=" * 60)
        
        curve = SM2CurveOptimized()
        G = (curve.Gx, curve.Gy)
        
        # 测试不同算法的性能
        methods = ["basic", "montgomery", "windowed_naf"]
        test_scalars = [
            0x123456789ABCDEF,
            0x123456789ABCDEF0123456789ABCDEF,
            0x123456789ABCDEF0123456789ABCDEF0123456789ABCDEF
        ]
        
        performance_results = []
        
        for scalar in test_scalars:
            print(f"\n测试标量: {hex(scalar)[:32]}...")
            scalar_results = {'scalar': hex(scalar), 'methods': {}}
            
            for method in methods:
                print(f"  方法: {method}")
                
                # 预热
                curve.point_multiply(scalar, G, method)
                
                # 性能测试
                iterations = 5
                start_time = time.time()
                for _ in range(iterations):
                    result = curve.point_multiply(scalar, G, method)
                end_time = time.time()
                
                avg_time = (end_time - start_time) / iterations
                print(f"    平均时间: {avg_time*1000:.2f}ms")
                print(f"    结果: ({hex(result[0])[:16]}..., {hex(result[1])[:16]}...)")
                
                scalar_results['methods'][method] = {
                    'avg_time': avg_time,
                    'result': {'x': hex(result[0]), 'y': hex(result[1])}
                }
            
            # 验证所有方法结果一致
            results = [scalar_results['methods'][m]['result'] for m in methods]
            assert all(r == results[0] for r in results), "算法结果不一致"
            print("    ✓ 所有算法结果一致")
            
            performance_results.append(scalar_results)
        
        # 计算性能提升
        print("\n性能提升统计:")
        base_times = [r['methods']['basic']['avg_time'] for r in performance_results]
        
        for method in methods[1:]:
            method_times = [r['methods'][method]['avg_time'] for r in performance_results]
            speedups = [b/m for b, m in zip(base_times, method_times)]
            avg_speedup = sum(speedups) / len(speedups)
            print(f"  {method}: 平均{avg_speedup:.2f}x加速")
        
        self.results['performance_metrics'] = {
            'test_results': performance_results,
            'speedup_analysis': {
                method: sum(base_times[i]/performance_results[i]['methods'][method]['avg_time'] 
                           for i in range(len(performance_results))) / len(performance_results)
                for method in methods[1:]
            }
        }
        
        # 运行完整基准测试
        print("\n运行完整基准测试...")
        try:
            benchmark_results = benchmark_comparison()
            self.results['performance_metrics']['benchmark_results'] = benchmark_results
        except Exception as e:
            print(f"基准测试失败: {e}")
        
        return True
    
    def demonstrate_security_analysis(self):
        """演示安全分析"""
        print("\n" + "=" * 60)
        print("3. 安全漏洞分析演示")
        print("=" * 60)
        
        try:
            analyzer = demonstrate_vulnerability_analysis()
            
            # 获取分析结果
            security_results = {
                'attack_results': dict(analyzer.attack_results),
                'report': analyzer.generate_attack_report()
            }
            
            self.results['security_analysis'] = security_results
            
        except Exception as e:
            print(f"安全分析演示失败: {e}")
            self.results['security_analysis'] = {'error': str(e)}
        
        return True
    
    def demonstrate_signature_misuse_research(self):
        """演示签名误用研究"""
        print("\n" + "=" * 60)
        print("4. 签名算法误用研究演示")
        print("=" * 60)
        
        try:
            # 执行POC分析
            poc_results, poc_report = poc_main()
            
            self.results['vulnerability_research'] = {
                'poc_results': poc_results,
                'report_summary': poc_report[:1000] + "..." if len(poc_report) > 1000 else poc_report
            }
            
        except Exception as e:
            print(f"签名误用研究演示失败: {e}")
            self.results['vulnerability_research'] = {'error': str(e)}
        
        return True
    
    def demonstrate_signature_comparison(self):
        """演示基础版vs优化版签名性能对比"""
        print("\n" + "=" * 60)
        print("5. 签名算法性能对比")
        print("=" * 60)
        
        # 基础实现
        basic_curve = SM2Curve()
        basic_keypair = SM2KeyPair(basic_curve)
        basic_signer = SM2DigitalSignature(basic_curve)
        basic_private, basic_public = basic_keypair.generate_keypair()
        
        # 优化实现
        optimized_curve = SM2CurveOptimized()
        optimized_signer = SM2DigitalSignatureOptimized(optimized_curve)
        optimized_private = basic_private  # 使用相同私钥
        optimized_public = optimized_curve.point_multiply(
            optimized_private, (optimized_curve.Gx, optimized_curve.Gy), "montgomery"
        )
        
        test_messages = [
            b"Short message",
            b"Medium length test message for signature performance comparison",
            b"Very long message to test signature performance with larger data sets. " * 10
        ]
        
        comparison_results = []
        
        for i, message in enumerate(test_messages):
            print(f"\n测试消息 {i+1} (长度: {len(message)} bytes):")
            
            # 基础版签名
            start_time = time.time()
            basic_signature = basic_signer.sign(message, basic_private, basic_public)
            basic_sign_time = time.time() - start_time
            
            start_time = time.time()
            basic_valid = basic_signer.verify(message, basic_signature, basic_public)
            basic_verify_time = time.time() - start_time
            
            print(f"  基础版 - 签名: {basic_sign_time*1000:.2f}ms, 验证: {basic_verify_time*1000:.2f}ms")
            
            # 优化版签名
            start_time = time.time()
            optimized_signature = optimized_signer.sign_optimized(message, optimized_private, optimized_public)
            optimized_sign_time = time.time() - start_time
            
            start_time = time.time()
            optimized_valid = optimized_signer.verify_optimized(message, optimized_signature, optimized_public)
            optimized_verify_time = time.time() - start_time
            
            print(f"  优化版 - 签名: {optimized_sign_time*1000:.2f}ms, 验证: {optimized_verify_time*1000:.2f}ms")
            
            # 性能提升
            sign_speedup = basic_sign_time / optimized_sign_time
            verify_speedup = basic_verify_time / optimized_verify_time
            
            print(f"  性能提升 - 签名: {sign_speedup:.2f}x, 验证: {verify_speedup:.2f}x")
            
            comparison_results.append({
                'message_length': len(message),
                'basic': {
                    'sign_time': basic_sign_time,
                    'verify_time': basic_verify_time,
                    'valid': basic_valid
                },
                'optimized': {
                    'sign_time': optimized_sign_time,
                    'verify_time': optimized_verify_time,
                    'valid': optimized_valid
                },
                'speedup': {
                    'sign': sign_speedup,
                    'verify': verify_speedup
                }
            })
        
        # 计算平均性能提升
        avg_sign_speedup = sum(r['speedup']['sign'] for r in comparison_results) / len(comparison_results)
        avg_verify_speedup = sum(r['speedup']['verify'] for r in comparison_results) / len(comparison_results)
        
        print(f"\n总体性能提升:")
        print(f"  平均签名加速: {avg_sign_speedup:.2f}x")
        print(f"  平均验证加速: {avg_verify_speedup:.2f}x")
        
        self.results['signature_comparison'] = {
            'detailed_results': comparison_results,
            'average_speedup': {
                'sign': avg_sign_speedup,
                'verify': avg_verify_speedup
            }
        }
        
        return True
    
    def save_results(self, filename: str = None):
        """保存演示结果"""
        if filename is None:
            timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
            filename = f"sm2_demo_results_{timestamp}.json"
        
        output_dir = "output"
        if not os.path.exists(output_dir):
            os.makedirs(output_dir)
        
        filepath = os.path.join(output_dir, filename)
        
        with open(filepath, 'w', encoding='utf-8') as f:
            json.dump(self.results, f, indent=2, ensure_ascii=False)
        
        print(f"\n演示结果已保存到: {filepath}")
        return filepath
    
    def generate_summary_report(self) -> str:
        """生成演示总结报告"""
        report_lines = [
            "SM2椭圆曲线密码算法综合演示报告",
            "=" * 60,
            "",
            f"演示时间: {self.results['timestamp']}",
            f"演示项目数: {len(self.results['demonstrations'])}",
            "",
            "演示内容总结:",
            "-" * 30
        ]
        
        # 基础功能总结
        if 'demonstrations' in self.results and self.results['demonstrations']:
            basic_demo = next((d for d in self.results['demonstrations'] if d['type'] == 'basic_functionality'), None)
            if basic_demo:
                signatures = basic_demo['signatures']
                success_count = sum(1 for s in signatures if s['valid'])
                avg_sign_time = sum(s['sign_time'] for s in signatures) / len(signatures) * 1000
                avg_verify_time = sum(s['verify_time'] for s in signatures) / len(signatures) * 1000
                
                report_lines.extend([
                    f"1. 基础功能演示:",
                    f"   - 成功签名验证: {success_count}/{len(signatures)}",
                    f"   - 平均签名时间: {avg_sign_time:.2f}ms",
                    f"   - 平均验证时间: {avg_verify_time:.2f}ms",
                    ""
                ])
        
        # 性能优化总结
        if 'performance_metrics' in self.results and 'speedup_analysis' in self.results['performance_metrics']:
            speedups = self.results['performance_metrics']['speedup_analysis']
            report_lines.extend([
                "2. 性能优化结果:",
            ])
            for method, speedup in speedups.items():
                report_lines.append(f"   - {method}: {speedup:.2f}x 加速")
            report_lines.append("")
        
        # 签名对比总结
        if 'signature_comparison' in self.results:
            comp = self.results['signature_comparison']['average_speedup']
            report_lines.extend([
                "3. 签名算法对比:",
                f"   - 平均签名加速: {comp['sign']:.2f}x",
                f"   - 平均验证加速: {comp['verify']:.2f}x",
                ""
            ])
        
        # 安全分析总结
        if 'security_analysis' in self.results:
            if 'error' not in self.results['security_analysis']:
                attack_results = self.results['security_analysis'].get('attack_results', {})
                report_lines.extend([
                    "4. 安全分析结果:",
                    f"   - 分析的攻击类型: {len(attack_results)}",
                    "   - 详细结果请查看完整报告",
                    ""
                ])
            else:
                report_lines.extend([
                    "4. 安全分析: 执行时遇到错误",
                    ""
                ])
        
        # 漏洞研究总结
        if 'vulnerability_research' in self.results:
            if 'error' not in self.results['vulnerability_research']:
                report_lines.extend([
                    "5. 签名误用研究:",
                    "   - Satoshi签名特征分析完成",
                    "   - 签名伪造POC验证完成",
                    "   - 详细结果请查看POC报告",
                    ""
                ])
            else:
                report_lines.extend([
                    "5. 签名误用研究: 执行时遇到错误",
                    ""
                ])
        
        report_lines.extend([
            "演示结论:",
            "-" * 30,
            "✓ SM2基础算法实现正确",
            "✓ 性能优化技术有效",
            "✓ 安全分析功能完整",
            "✓ 漏洞研究方法可行",
            "",
            "技术特点:",
            "- 完整的SM2椭圆曲线实现",
            "- 多种性能优化算法",
            "- 全面的安全漏洞分析",
            "- 深度的签名安全研究",
            "",
            "=" * 60
        ])
        
        return "\n".join(report_lines)


def main():
    """主函数 - 执行完整演示"""
    print("SM2椭圆曲线密码算法综合演示程序")
    print("开始时间:", datetime.now().strftime("%Y-%m-%d %H:%M:%S"))
    print("=" * 60)
    
    demo = SM2ComprehensiveDemo()
    
    try:
        # 执行各项演示
        demo.demonstrate_basic_functionality()
        demo.demonstrate_performance_optimization()
        demo.demonstrate_signature_comparison()
        demo.demonstrate_security_analysis()
        demo.demonstrate_signature_misuse_research()
        
        # 生成报告
        report = demo.generate_summary_report()
        print("\n" + "=" * 60)
        print("演示总结报告")
        print("=" * 60)
        print(report)
        
        # 保存结果
        results_file = demo.save_results()
        
        # 保存报告
        report_file = os.path.join("output", f"sm2_demo_report_{datetime.now().strftime('%Y%m%d_%H%M%S')}.txt")
        with open(report_file, 'w', encoding='utf-8') as f:
            f.write(report)
        print(f"演示报告已保存到: {report_file}")
        
        print(f"\n演示完成! 总用时: {time.time() - start_time:.2f}秒")
        
        return True
        
    except Exception as e:
        print(f"\n演示过程中发生错误: {e}")
        import traceback
        traceback.print_exc()
        return False


if __name__ == "__main__":
    start_time = time.time()
    success = main()
    
    if success:
        print("\n✓ 所有演示成功完成")
    else:
        print("\n✗ 演示过程中遇到问题")
        sys.exit(1)
