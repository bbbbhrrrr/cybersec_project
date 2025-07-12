"""
项目结果分析脚本
"""

import sys
import os
sys.path.append('src')

from watermark import WatermarkSystem
from attacks import AttackTestSuite
from utils import ImageUtils, QualityMetrics, WatermarkEvaluator
import numpy as np

def analyze_watermark_strength():
    """分析水印强度对性能的影响"""
    print("=== 水印强度影响分析 ===")
    
    # 创建测试图像
    test_img = ImageUtils.create_test_image(256, 256)
    ImageUtils.save_image(test_img, 'samples/analysis_test.png')
    
    ws = WatermarkSystem()
    test_text = 'Test2025'
    results = {}
    
    for strength in [20, 30, 40, 50]:
        ws.quantization_factor = strength
        watermarked = ws.embed_watermark('samples/analysis_test.png', test_text, f'output/test_strength_{strength}.png')
        
        # 计算质量指标
        original = ImageUtils.load_image('samples/analysis_test.png')
        psnr = QualityMetrics.calculate_psnr(original, watermarked)
        ssim = QualityMetrics.calculate_ssim(original, watermarked)
        
        # 测试提取
        extracted = ws.extract_watermark(f'output/test_strength_{strength}.png')
        accuracy = WatermarkEvaluator.calculate_extraction_accuracy(test_text, extracted)
        
        results[strength] = {'psnr': psnr, 'ssim': ssim, 'accuracy': accuracy, 'extracted': extracted}
        print(f'强度{strength}: PSNR={psnr:.1f}dB, SSIM={ssim:.3f}, 准确率={accuracy:.1%}, 提取="{extracted}"')
    
    return results

def analyze_image_types():
    """分析不同图像类型的水印性能"""
    print("\n=== 不同图像类型测试 ===")
    
    ws = WatermarkSystem(quantization_factor=35)
    original_text = 'Copyright 2025 bbbbhrrrr Digital Watermark Test'
    image_types = ['lena', 'geometric', 'text', 'random']
    
    results = {}
    for img_type in image_types:
        try:
            extracted = ws.extract_watermark(f'output/{img_type}_watermarked.png')
            accuracy = WatermarkEvaluator.calculate_extraction_accuracy(original_text, extracted)
            
            # 计算图像质量
            original = ImageUtils.load_image(f'samples/{img_type}.png')
            watermarked = ImageUtils.load_image(f'output/{img_type}_watermarked.png')
            psnr = QualityMetrics.calculate_psnr(original, watermarked)
            
            results[img_type] = {'accuracy': accuracy, 'psnr': psnr, 'extracted': extracted}
            print(f'{img_type}: 准确率={accuracy:.1%}, PSNR={psnr:.1f}dB')
        except Exception as e:
            print(f'{img_type}: 错误 - {e}')
            results[img_type] = {'error': str(e)}
    
    return results

def analyze_robustness_by_category():
    """按攻击类别分析鲁棒性"""
    print("\n=== 攻击类别鲁棒性分析 ===")
    
    ws = WatermarkSystem(quantization_factor=35)
    test_text = 'TestRobust'
    
    # 创建测试图像并嵌入水印
    test_img = ImageUtils.create_test_image(256, 256)
    ImageUtils.save_image(test_img, 'samples/robust_test.png')
    watermarked = ws.embed_watermark('samples/robust_test.png', test_text, 'output/robust_watermarked.png')
    
    # 测试关键攻击
    attack_suite = AttackTestSuite()
    attacks = {
        '几何变换': [
            ('horizontal_flip', {}),
            ('rotation', {'angle': 15}),
            ('translation', {'dx': 10, 'dy': 10})
        ],
        '图像处理': [
            ('gaussian_blur', {'kernel_size': 3, 'sigma': 1.0}),
            ('median_filter', {'kernel_size': 3}),
            ('sharpening', {})
        ],
        '噪声攻击': [
            ('gaussian_noise', {'mean': 0, 'std': 10}),
            ('salt_pepper_noise', {'noise_ratio': 0.01})
        ],
        '压缩攻击': [
            ('jpeg_compression', {'quality': 50}),
            ('jpeg_compression', {'quality': 30})
        ]
    }
    
    category_results = {}
    
    for category, attack_list in attacks.items():
        category_accuracies = []
        print(f"\n{category}:")
        
        for attack_name, params in attack_list:
            try:
                # 执行攻击
                attacked_path = attack_suite.run_single_attack(
                    'output/robust_watermarked.png', attack_name, 'output/analysis_attacks', **params
                )
                
                # 提取水印
                extracted = ws.extract_watermark(attacked_path)
                accuracy = WatermarkEvaluator.calculate_extraction_accuracy(test_text, extracted)
                category_accuracies.append(accuracy)
                
                print(f"  {attack_name}: {accuracy:.1%}")
                
            except Exception as e:
                print(f"  {attack_name}: 失败 - {e}")
        
        if category_accuracies:
            avg_accuracy = np.mean(category_accuracies)
            category_results[category] = avg_accuracy
            print(f"  平均准确率: {avg_accuracy:.1%}")
    
    return category_results

def generate_analysis_report():
    """生成完整的分析报告"""
    print("开始项目结果分析...")
    print("=" * 60)
    
    # 运行各项分析
    strength_results = analyze_watermark_strength()
    image_results = analyze_image_types()
    robustness_results = analyze_robustness_by_category()
    
    # 生成总结报告
    print("\n" + "=" * 60)
    print("项目总结分析报告")
    print("=" * 60)
    
    # 基本性能分析
    successful_images = [name for name, result in image_results.items() if 'error' not in result]
    if successful_images:
        avg_accuracy = np.mean([image_results[name]['accuracy'] for name in successful_images])
        avg_psnr = np.mean([image_results[name]['psnr'] for name in successful_images])
        print(f"基本性能:")
        print(f"  处理成功率: {len(successful_images)}/4 图像类型")
        print(f"  平均提取准确率: {avg_accuracy:.1%}")
        print(f"  平均PSNR: {avg_psnr:.1f}dB")
    
    # 水印强度分析
    best_strength = max(strength_results.keys(), key=lambda k: strength_results[k]['accuracy'])
    print(f"\n最优参数:")
    print(f"  最佳水印强度: {best_strength}")
    print(f"  对应性能: PSNR={strength_results[best_strength]['psnr']:.1f}dB, 准确率={strength_results[best_strength]['accuracy']:.1%}")
    
    # 鲁棒性分析
    if robustness_results:
        best_category = max(robustness_results.keys(), key=lambda k: robustness_results[k])
        worst_category = min(robustness_results.keys(), key=lambda k: robustness_results[k])
        print(f"\n鲁棒性分析:")
        print(f"  最强抗攻击类型: {best_category} ({robustness_results[best_category]:.1%})")
        print(f"  最弱抗攻击类型: {worst_category} ({robustness_results[worst_category]:.1%})")
        print(f"  整体平均鲁棒性: {np.mean(list(robustness_results.values())):.1%}")
    
    # 技术评估
    print(f"\n技术评估:")
    if avg_psnr > 35:
        print("  ✅ 图像质量: 优秀 (PSNR > 35dB)")
    elif avg_psnr > 25:
        print("  ⚠️ 图像质量: 良好 (25-35dB)")
    else:
        print("  ❌ 图像质量: 需改进 (< 25dB)")
    
    if avg_accuracy > 0.8:
        print("  ✅ 提取准确率: 优秀 (> 80%)")
    elif avg_accuracy > 0.6:
        print("  ⚠️ 提取准确率: 良好 (60-80%)")
    else:
        print("  ❌ 提取准确率: 需改进 (< 60%)")
    
    if robustness_results and np.mean(list(robustness_results.values())) > 0.5:
        print("  ✅ 鲁棒性: 良好 (平均 > 50%)")
    else:
        print("  ⚠️ 鲁棒性: 一般 (平均 < 50%)")
    
    # 保存结果到文件
    with open('output/project_analysis_report.txt', 'w', encoding='utf-8') as f:
        f.write("数字水印系统项目分析报告\n")
        f.write("=" * 40 + "\n\n")
        f.write(f"测试日期: 2025-07-12\n\n")
        
        f.write("1. 基本性能测试结果:\n")
        for img_type, result in image_results.items():
            if 'error' not in result:
                f.write(f"  {img_type}: 准确率={result['accuracy']:.1%}, PSNR={result['psnr']:.1f}dB\n")
        
        f.write("\n2. 水印强度分析:\n")
        for strength, result in strength_results.items():
            f.write(f"  强度{strength}: PSNR={result['psnr']:.1f}dB, 准确率={result['accuracy']:.1%}\n")
        
        f.write("\n3. 鲁棒性测试结果:\n")
        for category, accuracy in robustness_results.items():
            f.write(f"  {category}: {accuracy:.1%}\n")
        
        f.write(f"\n4. 总体评估:\n")
        f.write(f"  平均图像质量: {avg_psnr:.1f}dB\n")
        f.write(f"  平均提取准确率: {avg_accuracy:.1%}\n")
        if robustness_results:
            f.write(f"  平均鲁棒性: {np.mean(list(robustness_results.values())):.1%}\n")
    
    print(f"\n详细分析报告已保存到: output/project_analysis_report.txt")
    
    return {
        'strength_results': strength_results,
        'image_results': image_results,
        'robustness_results': robustness_results
    }

if __name__ == "__main__":
    os.makedirs('output', exist_ok=True)
    results = generate_analysis_report()
