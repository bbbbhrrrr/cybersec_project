"""
水印鲁棒性测试模块

对嵌入水印的图像进行各种攻击测试，评估水印的鲁棒性
"""

import sys
import os
sys.path.append(os.path.join(os.path.dirname(__file__), '..', 'src'))

import numpy as np
from attacks import AttackTestSuite
from watermark import WatermarkSystem
from utils import ImageUtils, QualityMetrics, WatermarkEvaluator, Visualizer

class RobustnessTest:
 """鲁棒性测试类"""

 def __init__(self, watermark_system: WatermarkSystem = None):
 """初始化鲁棒性测试"""
 self.watermark_sys = watermark_system or WatermarkSystem(quantization_factor=30)
 self.attack_suite = AttackTestSuite()
 self.test_results = {}

 def run_single_robustness_test(self, watermarked_image_path: str,
 original_text: str, attack_name: str,
 output_dir: str = "output/robustness", **attack_params) -> dict:
 """运行单个鲁棒性测试"""
 try:
 # 执行攻击
 attacked_image_path = self.attack_suite.run_single_attack(
 watermarked_image_path, attack_name, output_dir, **attack_params
 )

 # 从攻击后的图像提取水印
 extracted_text = self.watermark_sys.extract_watermark(attacked_image_path)

 # 计算评估指标
 accuracy = WatermarkEvaluator.calculate_extraction_accuracy(
 original_text, extracted_text
 )
 similarity = WatermarkEvaluator.text_similarity(original_text, extracted_text)

 # 计算图像质量指标
 original_img = ImageUtils.load_image(watermarked_image_path)
 attacked_img = ImageUtils.load_image(attacked_image_path)
 psnr = QualityMetrics.calculate_psnr(original_img, attacked_img)
 ssim = QualityMetrics.calculate_ssim(original_img, attacked_img)

 result = {
 'attack_name': attack_name,
 'attack_params': attack_params,
 'attacked_image_path': attacked_image_path,
 'original_text': original_text,
 'extracted_text': extracted_text,
 'accuracy': accuracy,
 'similarity': similarity,
 'psnr': psnr,
 'ssim': ssim,
 'success': True
 }

 print(f" {attack_name}: 准确率={accuracy:.2%}, 相似度={similarity:.2%}, PSNR={psnr:.1f}dB")
 return result

 except Exception as e:
 result = {
 'attack_name': attack_name,
 'success': False,
 'error': str(e)
 }
 print(f" {attack_name}: 测试失败 - {e}")
 return result

 def run_comprehensive_test(self, watermarked_image_path: str,
 original_text: str, output_dir: str = "output/robustness") -> dict:
 """运行全面的鲁棒性测试"""
 print(f"开始全面鲁棒性测试...")
 print(f"原始水印文本: '{original_text}'")
 print("=" * 60)

 # 定义所有攻击测试
 attack_configs = {
 # 几何变换攻击
 'horizontal_flip': {},
 'vertical_flip': {},
 'rotation_15': {'angle': 15},
 'rotation_30': {'angle': 30},
 'rotation_45': {'angle': 45},
 'translation_small': {'dx': 10, 'dy': 10},
 'translation_large': {'dx': 30, 'dy': 25},

 # 裁剪攻击
 'center_crop_80': {'crop_ratio': 0.8},
 'center_crop_60': {'crop_ratio': 0.6},
 'random_crop_70': {'crop_ratio': 0.7},
 'random_crop_50': {'crop_ratio': 0.5},

 # 缩放攻击
 'scale_down_50': {'scale_factor': 0.5},
 'scale_down_75': {'scale_factor': 0.75},
 'scale_up_125': {'scale_factor': 1.25},
 'scale_up_150': {'scale_factor': 1.5},

 # 图像处理攻击
 'gaussian_blur_mild': {'kernel_size': 3, 'sigma': 1.0},
 'gaussian_blur_strong': {'kernel_size': 7, 'sigma': 2.0},
 'median_filter_3': {'kernel_size': 3},
 'median_filter_5': {'kernel_size': 5},

 # 噪声攻击
 'gaussian_noise_mild': {'mean': 0, 'std': 10},
 'gaussian_noise_strong': {'mean': 0, 'std': 25},
 'salt_pepper_mild': {'noise_ratio': 0.01},
 'salt_pepper_strong': {'noise_ratio': 0.03},

 # 亮度和对比度攻击
 'brightness_up_mild': {'factor': 1.2},
 'brightness_up_strong': {'factor': 1.5},
 'brightness_down_mild': {'factor': 0.8},
 'brightness_down_strong': {'factor': 0.6},
 'contrast_up_mild': {'factor': 1.3},
 'contrast_up_strong': {'factor': 1.6},
 'contrast_down_mild': {'factor': 0.7},
 'contrast_down_strong': {'factor': 0.5},

 # 压缩攻击
 'jpeg_high_quality': {'quality': 80},
 'jpeg_medium_quality': {'quality': 50},
 'jpeg_low_quality': {'quality': 20},

 # 其他攻击
 'histogram_equalization': {},
 'sharpening': {}
 }

 results = {}
 successful_tests = 0
 total_tests = len(attack_configs)

 for test_name, params in attack_configs.items():
 base_attack = test_name.split('_')[0]
 if hasattr(self.attack_suite.attacks, base_attack):
 result = self.run_single_robustness_test(
 watermarked_image_path, original_text, base_attack, output_dir, **params
 )
 results[test_name] = result
 if result.get('success', False):
 successful_tests += 1

 # 计算总体统计
 successful_results = [r for r in results.values() if r.get('success', False)]
 if successful_results:
 avg_accuracy = np.mean([r['accuracy'] for r in successful_results])
 avg_similarity = np.mean([r['similarity'] for r in successful_results])
 avg_psnr = np.mean([r['psnr'] for r in successful_results])
 avg_ssim = np.mean([r['ssim'] for r in successful_results])

 # 统计不同准确率级别的测试数量
 high_accuracy = len([r for r in successful_results if r['accuracy'] >= 0.8])
 medium_accuracy = len([r for r in successful_results if 0.5 <= r['accuracy'] < 0.8])
 low_accuracy = len([r for r in successful_results if r['accuracy'] < 0.5])

 else:
 avg_accuracy = avg_similarity = avg_psnr = avg_ssim = 0
 high_accuracy = medium_accuracy = low_accuracy = 0

 # 生成测试报告
 summary = {
 'total_tests': total_tests,
 'successful_tests': successful_tests,
 'success_rate': successful_tests / total_tests if total_tests > 0 else 0,
 'avg_accuracy': avg_accuracy,
 'avg_similarity': avg_similarity,
 'avg_psnr': avg_psnr,
 'avg_ssim': avg_ssim,
 'high_accuracy_count': high_accuracy,
 'medium_accuracy_count': medium_accuracy,
 'low_accuracy_count': low_accuracy,
 'detailed_results': results
 }

 self.print_summary_report(summary)
 return summary

 def print_summary_report(self, summary: dict):
 """打印测试总结报告"""
 print("\n" + "=" * 60)
 print("鲁棒性测试总结报告")
 print("=" * 60)
 print(f"总测试数量: {summary['total_tests']}")
 print(f"成功测试数量: {summary['successful_tests']}")
 print(f"成功率: {summary['success_rate']:.1%}")
 print(f"平均提取准确率: {summary['avg_accuracy']:.2%}")
 print(f"平均文本相似度: {summary['avg_similarity']:.2%}")
 print(f"平均PSNR: {summary['avg_psnr']:.1f} dB")
 print(f"平均SSIM: {summary['avg_ssim']:.3f}")
 print()
 print("准确率分布:")
 print(f" 高准确率 (≥80%): {summary['high_accuracy_count']} 个测试")
 print(f" 中等准确率 (50-80%): {summary['medium_accuracy_count']} 个测试")
 print(f" 低准确率 (<50%): {summary['low_accuracy_count']} 个测试")

 # 按类别统计
 categories = {
 '几何变换': ['horizontal_flip', 'vertical_flip', 'rotation', 'translation'],
 '裁剪攻击': ['crop', 'random_crop'],
 '缩放攻击': ['scale'],
 '图像处理': ['blur', 'filter'],
 '噪声攻击': ['noise', 'salt_pepper'],
 '亮度对比度': ['brightness', 'contrast'],
 '压缩攻击': ['jpeg'],
 '其他攻击': ['histogram', 'sharpening']
 }

 print("\n按攻击类别统计:")
 for category, keywords in categories.items():
 category_results = []
 for test_name, result in summary['detailed_results'].items():
 if any(keyword in test_name for keyword in keywords) and result.get('success'):
 category_results.append(result['accuracy'])

 if category_results:
 avg_acc = np.mean(category_results)
 print(f" {category}: {avg_acc:.1%} (基于 {len(category_results)} 个测试)")

 def save_detailed_report(self, summary: dict, output_path: str = "output/robustness_report.txt"):
 """保存详细测试报告到文件"""
 os.makedirs(os.path.dirname(output_path), exist_ok=True)

 with open(output_path, 'w', encoding='utf-8') as f:
 f.write("数字水印鲁棒性测试详细报告\n")
 f.write("=" * 50 + "\n\n")

 # 总体统计
 f.write(f"测试时间: {os.getctime()}\n")
 f.write(f"总测试数量: {summary['total_tests']}\n")
 f.write(f"成功测试数量: {summary['successful_tests']}\n")
 f.write(f"成功率: {summary['success_rate']:.1%}\n")
 f.write(f"平均提取准确率: {summary['avg_accuracy']:.2%}\n")
 f.write(f"平均文本相似度: {summary['avg_similarity']:.2%}\n")
 f.write(f"平均PSNR: {summary['avg_psnr']:.1f} dB\n")
 f.write(f"平均SSIM: {summary['avg_ssim']:.3f}\n\n")

 # 详细结果
 f.write("详细测试结果:\n")
 f.write("-" * 50 + "\n")

 for test_name, result in summary['detailed_results'].items():
 if result.get('success', False):
 f.write(f"测试: {test_name}\n")
 f.write(f" 参数: {result.get('attack_params', {})}\n")
 f.write(f" 原始文本: {result['original_text']}\n")
 f.write(f" 提取文本: {result['extracted_text']}\n")
 f.write(f" 准确率: {result['accuracy']:.2%}\n")
 f.write(f" 相似度: {result['similarity']:.2%}\n")
 f.write(f" PSNR: {result['psnr']:.1f} dB\n")
 f.write(f" SSIM: {result['ssim']:.3f}\n")
 f.write("\n")
 else:
 f.write(f"测试: {test_name} - 失败\n")
 f.write(f" 错误: {result.get('error', 'Unknown error')}\n\n")

 print(f"详细报告已保存到: {output_path}")

def main():
 """运行鲁棒性测试示例"""
 print("开始水印鲁棒性测试...")

 # 创建目录
 os.makedirs("samples", exist_ok=True)
 os.makedirs("output", exist_ok=True)

 # 创建测试图像
 test_img = ImageUtils.create_test_image(512, 512)
 original_path = "samples/test_robustness.png"
 watermarked_path = "output/watermarked_robustness.png"

 ImageUtils.save_image(test_img, original_path)

 # 创建水印系统并嵌入水印
 watermark_sys = WatermarkSystem(quantization_factor=35)
 test_text = "Copyright 2025 Robustness Test"

 try:
 watermarked = watermark_sys.embed_watermark(
 original_path, test_text, watermarked_path
 )

 print(f"水印已嵌入: {test_text}")

 # 运行鲁棒性测试
 robustness_test = RobustnessTest(watermark_sys)
 summary = robustness_test.run_comprehensive_test(watermarked_path, test_text)

 # 保存详细报告
 robustness_test.save_detailed_report(summary)

 print("\n鲁棒性测试完成！")

 except Exception as e:
 print(f"鲁棒性测试过程中出现错误: {e}")

if __name__ == "__main__":
 main()
