"""
数字水印系统演示脚本

展示完整的水印嵌入、攻击测试和鲁棒性评估流程
"""

import sys
import os
import numpy as np

# 添加src目录到路径
sys.path.append(os.path.join(os.path.dirname(__file__), 'src'))

from watermark import WatermarkSystem
from attacks import AttackTestSuite
from utils import ImageUtils, QualityMetrics, WatermarkEvaluator
from tests.test_robustness import RobustnessTest

def create_sample_images():
 """创建示例图片"""
 print("创建示例图片...")

 # 创建目录
 os.makedirs("samples", exist_ok=True)

 # 创建多种测试图片
 images = {
 "lena": create_lena_like_image(),
 "geometric": create_geometric_pattern(),
 "text": create_text_image(),
 "random": create_random_texture()
 }

 for name, image in images.items():
 path = f"samples/{name}.png"
 ImageUtils.save_image(image, path)
 print(f" 创建 {path}")

 return list(images.keys())

def create_lena_like_image(size=512):
 """创建类似Lena的测试图像"""
 img = np.zeros((size, size), dtype=np.uint8)

 # 创建脸部轮廓
 center = size // 2
 for y in range(size):
 for x in range(size):
 # 椭圆脸型
 dx, dy = x - center, y - center
 if (dx**2 / (center*0.8)**2) + (dy**2 / (center*0.9)**2) <= 1:
 img[y, x] = 180 + int(30 * np.sin(x*0.1) * np.cos(y*0.1))

 # 添加特征
 # 眼睛
 cv2.circle(img, (center-60, center-40), 15, 50, -1)
 cv2.circle(img, (center+60, center-40), 15, 50, -1)

 # 鼻子
 cv2.ellipse(img, (center, center+10), (8, 20), 0, 0, 360, 120, -1)

 # 嘴巴
 cv2.ellipse(img, (center, center+60), (30, 15), 0, 0, 180, 100, 2)

 return img

def create_geometric_pattern(size=512):
 """创建几何图案"""
 img = np.ones((size, size), dtype=np.uint8) * 128

 # 添加网格
 for i in range(0, size, 32):
 img[i:i+2, :] = 200
 img[:, i:i+2] = 200

 # 添加圆形图案
 for i in range(4):
 for j in range(4):
 center_x = size // 8 + i * size // 4
 center_y = size // 8 + j * size // 4
 radius = size // 16
 cv2.circle(img, (center_x, center_y), radius, 80, -1)

 return img

def create_text_image(size=512):
 """创建文字图案"""
 img = np.ones((size, size), dtype=np.uint8) * 240

 # 添加文字
 texts = ["WATERMARK", "TEST", "2025", "PROJECT"]
 for i, text in enumerate(texts):
 y_pos = size // 6 + i * size // 5
 cv2.putText(img, text, (50, y_pos), cv2.FONT_HERSHEY_SIMPLEX, 1.5, 0, 3)

 return img

def create_random_texture(size=512):
 """创建随机纹理"""
 # 创建Perlin噪声风格的纹理
 img = np.random.randint(0, 256, (size, size), dtype=np.uint8)

 # 添加高斯滤波使其更平滑
 img = cv2.GaussianBlur(img, (15, 15), 5)

 return img

def demonstrate_basic_functionality():
 """演示基本功能"""
 print("\n" + "="*60)
 print("基本功能演示")
 print("="*60)

 # 创建水印系统
 watermark_sys = WatermarkSystem(quantization_factor=30)

 # 测试不同的图片
 sample_images = ["lena", "geometric", "text", "random"]
 watermark_text = "Copyright 2025 bbbbhrrrr Digital Watermark Test"

 results = {}

 for img_name in sample_images:
 print(f"\n处理图片: {img_name}.png")

 try:
 # 嵌入水印
 original_path = f"samples/{img_name}.png"
 watermarked_path = f"output/{img_name}_watermarked.png"

 os.makedirs("output", exist_ok=True)

 watermarked = watermark_sys.embed_watermark(
 original_path, watermark_text, watermarked_path
 )

 # 计算质量指标
 original_img = ImageUtils.load_image(original_path)
 psnr = QualityMetrics.calculate_psnr(original_img, watermarked)
 ssim = QualityMetrics.calculate_ssim(original_img, watermarked)

 # 提取水印
 extracted = watermark_sys.extract_watermark(watermarked_path)
 accuracy = WatermarkEvaluator.calculate_extraction_accuracy(watermark_text, extracted)

 results[img_name] = {
 'psnr': psnr,
 'ssim': ssim,
 'accuracy': accuracy,
 'extracted_text': extracted
 }

 print(f" 嵌入成功 - PSNR: {psnr:.1f}dB, SSIM: {ssim:.3f}")
 print(f" 提取成功 - 准确率: {accuracy:.1%}")
 print(f" 原始: {watermark_text[:50]}...")
 print(f" 提取: {extracted[:50]}...")

 except Exception as e:
 print(f" 处理失败: {e}")
 results[img_name] = {'error': str(e)}

 return results

def demonstrate_robustness_testing():
 """演示鲁棒性测试"""
 print("\n" + "="*60)
 print("鲁棒性测试演示")
 print("="*60)

 # 选择最佳测试图片（基于基本功能测试结果）
 test_image = "lena" # 通常人脸图像有较好的水印性能
 watermark_text = "Robustness Test 2025"

 # 创建水印系统并嵌入水印
 watermark_sys = WatermarkSystem(quantization_factor=35)

 original_path = f"samples/{test_image}.png"
 watermarked_path = f"output/{test_image}_robustness.png"

 try:
 print(f"使用图片: {test_image}.png")
 print(f"水印文本: {watermark_text}")

 # 嵌入水印
 watermarked = watermark_sys.embed_watermark(
 original_path, watermark_text, watermarked_path
 )

 # 运行鲁棒性测试
 robustness_test = RobustnessTest(watermark_sys)

 print("\n开始运行部分鲁棒性测试...")

 # 运行关键攻击测试
 key_attacks = [
 ('rotation', {'angle': 15}),
 ('gaussian_blur', {'kernel_size': 5, 'sigma': 1.5}),
 ('jpeg_compression', {'quality': 50}),
 ('crop_attack', {'crop_ratio': 0.8}),
 ('gaussian_noise', {'mean': 0, 'std': 15})
 ]

 attack_results = {}
 for attack_name, params in key_attacks:
 result = robustness_test.run_single_robustness_test(
 watermarked_path, watermark_text, attack_name,
 "output/demo_attacks", **params
 )
 attack_results[attack_name] = result

 # 统计结果
 successful_attacks = [r for r in attack_results.values() if r.get('success', False)]
 if successful_attacks:
 avg_accuracy = np.mean([r['accuracy'] for r in successful_attacks])
 avg_psnr = np.mean([r['psnr'] for r in successful_attacks])

 print(f"\n关键攻击测试结果:")
 print(f"成功测试: {len(successful_attacks)}/{len(key_attacks)}")
 print(f"平均准确率: {avg_accuracy:.1%}")
 print(f"平均PSNR: {avg_psnr:.1f}dB")

 # 显示具体结果
 for attack_name, result in attack_results.items():
 if result.get('success'):
 print(f" {attack_name}: 准确率={result['accuracy']:.1%}, PSNR={result['psnr']:.1f}dB")

 return attack_results

 except Exception as e:
 print(f"鲁棒性测试失败: {e}")
 return {}

def generate_demo_report(basic_results, robustness_results):
 """生成演示报告"""
 print("\n" + "="*60)
 print("系统性能总结报告")
 print("="*60)

 # 基本功能总结
 successful_basic = [name for name, result in basic_results.items() if 'error' not in result]
 if successful_basic:
 avg_psnr = np.mean([basic_results[name]['psnr'] for name in successful_basic])
 avg_ssim = np.mean([basic_results[name]['ssim'] for name in successful_basic])
 avg_accuracy = np.mean([basic_results[name]['accuracy'] for name in successful_basic])

 print(f"基本功能测试:")
 print(f" 成功处理图片: {len(successful_basic)}/4")
 print(f" 平均PSNR: {avg_psnr:.1f}dB")
 print(f" 平均SSIM: {avg_ssim:.3f}")
 print(f" 平均提取准确率: {avg_accuracy:.1%}")

 # 鲁棒性测试总结
 if robustness_results:
 successful_robustness = [r for r in robustness_results.values() if r.get('success', False)]
 if successful_robustness:
 avg_rob_accuracy = np.mean([r['accuracy'] for r in successful_robustness])
 avg_rob_psnr = np.mean([r['psnr'] for r in successful_robustness])

 print(f"\n鲁棒性测试:")
 print(f" 成功攻击测试: {len(successful_robustness)}/5")
 print(f" 平均提取准确率: {avg_rob_accuracy:.1%}")
 print(f" 平均攻击后PSNR: {avg_rob_psnr:.1f}dB")

 # 系统评价
 print(f"\n系统总体评价:")
 if successful_basic and len(successful_basic) >= 3:
 print(" 基本功能: 优秀")
 else:
 print(" 基本功能: 需要改进")

 if robustness_results and len([r for r in robustness_results.values() if r.get('success', False) and r.get('accuracy', 0) > 0.7]) >= 3:
 print(" 鲁棒性: 良好")
 else:
 print(" 鲁棒性: 一般")

 print("\n演示完成！详细结果已保存在output目录中。")

def main():
 """主演示函数"""
 print("数字图像水印系统演示")
 print("="*60)

 try:
 # 导入OpenCV
 global cv2
 import cv2

 # 第一步：创建示例图片
 sample_names = create_sample_images()

 # 第二步：演示基本功能
 basic_results = demonstrate_basic_functionality()

 # 第三步：演示鲁棒性测试
 robustness_results = demonstrate_robustness_testing()

 # 第四步：生成报告
 generate_demo_report(basic_results, robustness_results)

 except ImportError as e:
 print(f"依赖库导入失败: {e}")
 print("请安装所需依赖: pip install opencv-python numpy pillow matplotlib")
 except Exception as e:
 print(f"演示过程中出现错误: {e}")

if __name__ == "__main__":
 main()
