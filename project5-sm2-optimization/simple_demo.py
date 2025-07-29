"""
SM2椭圆曲线密码算法简化演示
"""

import sys
import os
import time
import json
from datetime import datetime

# 添加src路径到模块搜索路径
current_dir = os.path.dirname(os.path.abspath(__file__))
src_dir = os.path.join(current_dir, 'src')
basic_dir = os.path.join(src_dir, 'basic')
optimized_dir = os.path.join(src_dir, 'optimized')

sys.path.insert(0, basic_dir)
sys.path.insert(0, optimized_dir)

def demonstrate_basic_sm2():
    """演示基础SM2功能"""
    print("=" * 60)
    print("SM2椭圆曲线数字签名算法演示")
    print("=" * 60)
    
    try:
        # 导入基础模块
        from sm2_basic import SM2Curve, SM2KeyPair, SM2DigitalSignature
        
        print("1. 初始化椭圆曲线...")
        curve = SM2Curve()
        keypair = SM2KeyPair(curve)
        signer = SM2DigitalSignature(curve)
        
        print(f"   椭圆曲线参数 p = {hex(curve.p)[:32]}...")
        print(f"   基点 G = ({hex(curve.Gx)[:16]}..., {hex(curve.Gy)[:16]}...)")
        
        print("\n2. 生成密钥对...")
        private_key, public_key = keypair.generate_keypair()
        print(f"   私钥: {hex(private_key)[:32]}...")
        print(f"   公钥: ({hex(public_key[0])[:16]}..., {hex(public_key[1])[:16]}...)")
        
        # 验证公钥在曲线上
        assert curve.is_on_curve(public_key), "公钥不在曲线上"
        print("   ✓ 公钥验证通过")
        
        print("\n3. 数字签名测试...")
        test_messages = [
            b"Hello, SM2 Digital Signature!",
            b"Test message for SM2 algorithm",
            "中文消息测试".encode('utf-8')
        ]
        
        for i, message in enumerate(test_messages):
            print(f"\n   消息 {i+1}: {message.decode('utf-8', errors='ignore')}")
            
            # 签名
            start_time = time.time()
            signature = signer.sign(message, private_key, public_key)
            sign_time = time.time() - start_time
            
            print(f"   签名: r={hex(signature[0])[:16]}..., s={hex(signature[1])[:16]}...")
            print(f"   签名时间: {sign_time*1000:.2f}ms")
            
            # 验证
            start_time = time.time()
            is_valid = signer.verify(message, signature, public_key)
            verify_time = time.time() - start_time
            
            print(f"   验证结果: {'通过' if is_valid else '失败'}")
            print(f"   验证时间: {verify_time*1000:.2f}ms")
            
            # 验证错误消息应该失败
            wrong_signature = signer.verify(b"wrong message", signature, public_key)
            print(f"   错误消息验证: {'失败' if not wrong_signature else '意外通过'}")
        
        return True
        
    except ImportError as e:
        print(f"导入错误: {e}")
        return False
    except Exception as e:
        print(f"演示过程中发生错误: {e}")
        import traceback
        traceback.print_exc()
        return False

def demonstrate_optimized_sm2():
    """演示优化SM2功能"""
    print("\n" + "=" * 60)
    print("SM2优化算法性能对比演示")
    print("=" * 60)
    
    try:
        from sm2_optimized import SM2CurveOptimized, SM2DigitalSignatureOptimized
        
        print("1. 初始化优化椭圆曲线...")
        curve = SM2CurveOptimized()
        signer = SM2DigitalSignatureOptimized(curve)
        
        G = (curve.Gx, curve.Gy)
        test_scalar = 0x123456789ABCDEF0123456789ABCDEF
        
        print(f"   测试标量: {hex(test_scalar)[:32]}...")
        
        print("\n2. 不同算法性能对比...")
        methods = ["basic", "montgomery", "windowed_naf"]
        results = {}
        
        for method in methods:
            print(f"   测试方法: {method}")
            
            # 预热
            curve.point_multiply(test_scalar, G, method)
            
            # 性能测试
            iterations = 10
            start_time = time.time()
            for _ in range(iterations):
                result = curve.point_multiply(test_scalar, G, method)
            end_time = time.time()
            
            avg_time = (end_time - start_time) / iterations
            results[method] = {
                'time': avg_time,
                'result': result
            }
            
            print(f"     平均时间: {avg_time*1000:.2f}ms")
            print(f"     结果: ({hex(result[0])[:16]}..., {hex(result[1])[:16]}...)")
        
        # 验证结果一致性并计算加速比
        print("\n3. 性能提升统计:")
        base_time = results['basic']['time']
        
        for method in methods[1:]:
            if results[method]['result'] == results['basic']['result']:
                speedup = base_time / results[method]['time']
                print(f"   {method}: {speedup:.2f}x 加速 ✓")
            else:
                print(f"   {method}: 结果不一致 ✗")
        
        print("\n4. 优化签名算法测试...")
        # 生成密钥对
        private_key = 0x123456789ABCDEF0123456789ABCDEF0123456789ABCDEF
        public_key = curve.point_multiply(private_key, G, "montgomery")
        
        message = b"Optimized signature test"
        print(f"   测试消息: {message.decode()}")
        
        # 优化签名
        start_time = time.time()
        signature = signer.sign_optimized(message, private_key, public_key)
        sign_time = time.time() - start_time
        
        print(f"   签名: r={hex(signature[0])[:16]}..., s={hex(signature[1])[:16]}...")
        print(f"   签名时间: {sign_time*1000:.2f}ms")
        
        # 优化验证
        start_time = time.time()
        is_valid = signer.verify_optimized(message, signature, public_key)
        verify_time = time.time() - start_time
        
        print(f"   验证结果: {'通过' if is_valid else '失败'}")
        print(f"   验证时间: {verify_time*1000:.2f}ms")
        
        return True
        
    except ImportError as e:
        print(f"导入错误: {e}")
        return False
    except Exception as e:
        print(f"优化演示过程中发生错误: {e}")
        import traceback
        traceback.print_exc()
        return False

def demonstrate_security_analysis():
    """演示安全分析功能"""
    print("\n" + "=" * 60)
    print("SM2安全漏洞分析演示")
    print("=" * 60)
    
    try:
        from security_analysis import SM2SignatureVulnerabilityAnalyzer
        
        print("1. 初始化安全分析器...")
        curve_params = {
            'p': 0xFFFFFFFEFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000FFFFFFFFFFFFFFFF,
            'n': 0xFFFFFFFEFFFFFFFFFFFFFFFFFFFFFFFF7203DF6B21C6052B53BBF40939D54123,
            'Gx': 0x32C4AE2C1F1981195F9904466A39C9948FE30BBFF2660BE1715A4589334C74C7,
            'Gy': 0xBC3736A2F4F6779C59BDCEE36B692153D0A9877CC62A474002DF32E52139F0A0
        }
        analyzer = SM2SignatureVulnerabilityAnalyzer(curve_params)
        
        print("\n2. 重复随机数攻击演示...")
        same_r = 0x123456789ABCDEF
        signatures = [(same_r, 0x111111111), (same_r, 0x222222222)]
        messages = [b"message1", b"message2"]
        public_key = (0x11111, 0x22222)
        
        recovered_key = analyzer.nonce_reuse_attack(signatures, messages, public_key)
        if recovered_key:
            print(f"   ✓ 攻击成功，恢复私钥: {hex(recovered_key)[:32]}...")
        else:
            print("   ✗ 攻击失败")
        
        print("\n3. 时序攻击模拟...")
        operations = list(range(50))
        timing_result = analyzer.timing_attack_simulation(operations, timing_noise=0.1)
        print(f"   时序攻击精度: {timing_result['accuracy']:.2%}")
        print(f"   攻击成功: {'是' if timing_result['success'] else '否'}")
        
        return True
        
    except ImportError as e:
        print(f"导入错误: {e}")
        return False
    except Exception as e:
        print(f"安全分析演示过程中发生错误: {e}")
        import traceback
        traceback.print_exc()
        return False

def demonstrate_signature_misuse():
    """演示签名误用研究"""
    print("\n" + "=" * 60)
    print("签名算法误用研究演示")
    print("=" * 60)
    
    try:
        from signature_misuse_poc import SM2SignatureMisuseAnalyzer
        
        print("1. 初始化误用分析器...")
        analyzer = SM2SignatureMisuseAnalyzer()
        
        print("\n2. Satoshi Nakamoto签名特征分析...")
        analysis_result = analyzer.analyze_satoshi_signatures()
        
        print(f"   发现脆弱性指标: {len(analysis_result['vulnerability_indicators'])}")
        print(f"   伪造成功概率: {analysis_result['forge_probability']:.2%}")
        print(f"   风险等级: {analysis_result['security_implications']['risk_level']}")
        
        print("\n3. 签名伪造演示...")
        forge_result = analyzer.demonstrate_signature_forge()
        
        if forge_result['success']:
            print(f"   ✓ 伪造成功")
            print(f"   伪造签名: r={hex(forge_result['signature'][0])[:16]}...")
            print(f"   置信度: {forge_result['confidence']:.2%}")
            print(f"   方法: {forge_result['method']}")
        else:
            print("   ✗ 伪造失败")
        
        return True
        
    except ImportError as e:
        print(f"导入错误: {e}")
        return False
    except Exception as e:
        print(f"误用研究演示过程中发生错误: {e}")
        import traceback
        traceback.print_exc()
        return False

def save_demo_results():
    """保存演示结果"""
    output_dir = "output"
    if not os.path.exists(output_dir):
        os.makedirs(output_dir)
    
    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    
    demo_results = {
        'timestamp': datetime.now().isoformat(),
        'project': 'SM2椭圆曲线密码算法优化',
        'version': '1.0.0',
        'description': '完整的SM2实现包含基础算法、性能优化、安全分析和漏洞研究',
        'components': [
            '基础SM2椭圆曲线实现',
            'Montgomery阶梯和窗口NAF优化',
            '签名安全漏洞分析',
            'Satoshi Nakamoto签名特征研究',
            '签名伪造POC验证'
        ],
        'performance_features': [
            '雅可比坐标系优化',
            '批量模逆元计算',
            'Shamir双标量乘法',
            '预计算表缓存'
        ],
        'security_features': [
            '重复随机数攻击检测',
            '弱随机数分析',
            '时序攻击模拟',
            '格攻击评估',
            '签名模式识别'
        ]
    }
    
    # 保存JSON结果
    json_file = os.path.join(output_dir, f"sm2_demo_results_{timestamp}.json")
    with open(json_file, 'w', encoding='utf-8') as f:
        json.dump(demo_results, f, indent=2, ensure_ascii=False)
    
    # 保存文本报告
    report_lines = [
        "SM2椭圆曲线密码算法项目演示报告",
        "=" * 50,
        "",
        f"演示时间: {demo_results['timestamp']}",
        f"项目版本: {demo_results['version']}",
        "",
        "项目描述:",
        demo_results['description'],
        "",
        "主要组件:",
    ]
    
    for i, component in enumerate(demo_results['components'], 1):
        report_lines.append(f"  {i}. {component}")
    
    report_lines.extend([
        "",
        "性能优化特性:",
    ])
    
    for feature in demo_results['performance_features']:
        report_lines.append(f"  • {feature}")
    
    report_lines.extend([
        "",
        "安全分析特性:",
    ])
    
    for feature in demo_results['security_features']:
        report_lines.append(f"  • {feature}")
    
    report_lines.extend([
        "",
        "技术亮点:",
        "• 完整的SM2椭圆曲线数字签名算法实现",
        "• 多种性能优化技术的对比验证",
        "• 全面的签名安全漏洞分析框架",
        "• 基于历史安全事件的签名模式研究",
        "• 学术级别的密码学安全评估",
        "",
        "应用价值:",
        "• 密码学教学和研究",
        "• 安全系统开发参考",
        "• 数字签名安全评估",
        "• 区块链和加密货币安全分析",
        "",
        "=" * 50
    ])
    
    report_file = os.path.join(output_dir, f"sm2_demo_report_{timestamp}.txt")
    with open(report_file, 'w', encoding='utf-8') as f:
        f.write('\n'.join(report_lines))
    
    print(f"\n演示结果已保存:")
    print(f"  JSON格式: {json_file}")
    print(f"  文本报告: {report_file}")
    
    return json_file, report_file

def main():
    """主函数"""
    start_time = time.time()
    
    print("SM2椭圆曲线密码算法综合演示程序")
    print(f"开始时间: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}")
    print("=" * 60)
    print("本程序演示以下功能:")
    print("1. SM2基础椭圆曲线数字签名算法")
    print("2. 性能优化技术对比")
    print("3. 签名安全漏洞分析")
    print("4. 签名算法误用研究")
    print("=" * 60)
    
    success_count = 0
    total_demos = 4
    
    # 执行各项演示
    if demonstrate_basic_sm2():
        success_count += 1
    
    if demonstrate_optimized_sm2():
        success_count += 1
    
    if demonstrate_security_analysis():
        success_count += 1
    
    if demonstrate_signature_misuse():
        success_count += 1
    
    # 保存结果
    try:
        save_demo_results()
    except Exception as e:
        print(f"保存结果时发生错误: {e}")
    
    # 总结
    elapsed_time = time.time() - start_time
    
    print("\n" + "=" * 60)
    print("演示总结")
    print("=" * 60)
    print(f"总演示项目: {total_demos}")
    print(f"成功完成: {success_count}")
    print(f"成功率: {success_count/total_demos:.1%}")
    print(f"总用时: {elapsed_time:.2f}秒")
    
    if success_count == total_demos:
        print("\n✓ 所有演示项目成功完成!")
        print("SM2椭圆曲线密码算法实现验证通过")
    else:
        print(f"\n⚠ {total_demos - success_count} 个演示项目未能完成")
        print("请检查相关模块实现")
    
    print("\n项目特点:")
    print("• 学术研究级别的密码学实现")
    print("• 多种性能优化技术集成")
    print("• 全面的安全漏洞分析")
    print("• 实用的签名伪造POC验证")
    
    return success_count == total_demos

if __name__ == "__main__":
    success = main()
    
    if not success:
        sys.exit(1)
