"""
水印系统功能测试模块

测试水印嵌入和提取的基本功能
"""

import sys
import os
sys.path.append(os.path.join(os.path.dirname(__file__), '..', 'src'))

import unittest
import numpy as np
from watermark import WatermarkSystem
from utils import ImageUtils, QualityMetrics, WatermarkEvaluator

class TestWatermarkSystem(unittest.TestCase):
 """水印系统测试类"""

 def setUp(self):
 """测试初始化"""
 self.watermark_sys = WatermarkSystem(quantization_factor=30)
 self.test_image_path = "samples/test_watermark.png"
 self.watermarked_path = "output/test_watermarked.png"
 self.test_text = "Copyright 2025 Test"

 # 创建目录
 os.makedirs("samples", exist_ok=True)
 os.makedirs("output", exist_ok=True)

 # 创建测试图像
 test_img = ImageUtils.create_test_image(512, 512)
 ImageUtils.save_image(test_img, self.test_image_path)

 def tearDown(self):
 """测试清理"""
 # 清理测试文件
 for path in [self.test_image_path, self.watermarked_path]:
 if os.path.exists(path):
 try:
 os.remove(path)
 except:
 pass

 def test_text_to_binary_conversion(self):
 """测试文本和二进制转换"""
 text = "Hello"
 binary = self.watermark_sys.text_to_binary(text)
 converted_back = self.watermark_sys.binary_to_text(binary)

 self.assertTrue(text in converted_back)

 def test_watermark_embedding(self):
 """测试水印嵌入"""
 try:
 watermarked = self.watermark_sys.embed_watermark(
 self.test_image_path,
 self.test_text,
 self.watermarked_path
 )

 # 检查输出图像是否存在
 self.assertTrue(os.path.exists(self.watermarked_path))

 # 检查图像质量
 original = ImageUtils.load_image(self.test_image_path)
 psnr = QualityMetrics.calculate_psnr(original, watermarked)

 # PSNR应该大于25dB（基本质量要求）
 self.assertGreater(psnr, 25)

 except Exception as e:
 self.fail(f"水印嵌入失败: {e}")

 def test_watermark_extraction(self):
 """测试水印提取"""
 try:
 # 先嵌入水印
 self.watermark_sys.embed_watermark(
 self.test_image_path,
 self.test_text,
 self.watermarked_path
 )

 # 提取水印
 extracted = self.watermark_sys.extract_watermark(self.watermarked_path)

 # 计算准确率
 accuracy = WatermarkEvaluator.calculate_extraction_accuracy(
 self.test_text, extracted
 )

 # 准确率应该大于50%
 self.assertGreater(accuracy, 0.5)

 except Exception as e:
 self.fail(f"水印提取失败: {e}")

 def test_different_watermark_strengths(self):
 """测试不同水印强度"""
 strengths = [10, 30, 50]
 results = []

 for strength in strengths:
 try:
 watermarked = self.watermark_sys.embed_watermark(
 self.test_image_path,
 self.test_text,
 strength=strength
 )

 # 计算PSNR
 original = ImageUtils.load_image(self.test_image_path)
 psnr = QualityMetrics.calculate_psnr(original, watermarked)
 results.append((strength, psnr))

 except Exception as e:
 self.fail(f"强度 {strength} 测试失败: {e}")

 # 检查强度越大，PSNR越小（质量越差）
 psnrs = [result[1] for result in results]
 self.assertGreater(psnrs[0], psnrs[2]) # 强度10应该比强度50的PSNR高

 def test_empty_watermark(self):
 """测试空水印处理"""
 with self.assertRaises(Exception):
 self.watermark_sys.embed_watermark(
 self.test_image_path,
 "", # 空水印
 self.watermarked_path
 )

 def test_long_watermark(self):
 """测试长水印处理"""
 long_text = "This is a very long watermark text that might exceed the capacity" * 10

 # 应该抛出异常或正常处理
 try:
 self.watermark_sys.embed_watermark(
 self.test_image_path,
 long_text,
 self.watermarked_path
 )
 except ValueError as e:
 # 预期的容量不足错误
 self.assertIn("水印过长", str(e))

 def test_small_image(self):
 """测试小尺寸图像"""
 small_img = ImageUtils.create_test_image(64, 64)
 small_path = "samples/small_test.png"
 ImageUtils.save_image(small_img, small_path)

 try:
 watermarked = self.watermark_sys.embed_watermark(
 small_path,
 "Short", # 短水印
 strength=20
 )

 # 应该能够成功处理
 self.assertIsNotNone(watermarked)

 except Exception as e:
 self.fail(f"小图像处理失败: {e}")
 finally:
 if os.path.exists(small_path):
 os.remove(small_path)

class TestQualityMetrics(unittest.TestCase):
 """质量评估测试类"""

 def test_psnr_calculation(self):
 """测试PSNR计算"""
 # 相同图像的PSNR应该是无穷大
 img = np.random.randint(0, 256, (100, 100), dtype=np.uint8)
 psnr = QualityMetrics.calculate_psnr(img, img)
 self.assertEqual(psnr, float('inf'))

 # 不同图像的PSNR应该是有限值
 img2 = img + 10
 psnr = QualityMetrics.calculate_psnr(img, img2)
 self.assertIsFinite(psnr)
 self.assertGreater(psnr, 0)

 def test_ssim_calculation(self):
 """测试SSIM计算"""
 # 相同图像的SSIM应该接近1
 img = np.random.randint(0, 256, (100, 100), dtype=np.uint8)
 ssim = QualityMetrics.calculate_ssim(img, img)
 self.assertAlmostEqual(ssim, 1.0, places=2)

 # 不同图像的SSIM应该小于1
 img2 = np.random.randint(0, 256, (100, 100), dtype=np.uint8)
 ssim = QualityMetrics.calculate_ssim(img, img2)
 self.assertLess(ssim, 1.0)

class TestWatermarkEvaluator(unittest.TestCase):
 """水印评估测试类"""

 def test_extraction_accuracy(self):
 """测试提取准确率计算"""
 # 完全相同的文本
 accuracy = WatermarkEvaluator.calculate_extraction_accuracy(
 "Hello", "Hello"
 )
 self.assertEqual(accuracy, 1.0)

 # 部分相同的文本
 accuracy = WatermarkEvaluator.calculate_extraction_accuracy(
 "Hello World", "Hello"
 )
 self.assertGreater(accuracy, 0)
 self.assertLess(accuracy, 1.0)

 # 完全不同的文本
 accuracy = WatermarkEvaluator.calculate_extraction_accuracy(
 "Hello", "12345"
 )
 self.assertEqual(accuracy, 0.0)

 def test_text_similarity(self):
 """测试文本相似度计算"""
 # 相同文本
 similarity = WatermarkEvaluator.text_similarity("Hello", "Hello")
 self.assertEqual(similarity, 1.0)

 # 部分相同
 similarity = WatermarkEvaluator.text_similarity("Hello", "Hell")
 self.assertGreater(similarity, 0.5)

 # 完全不同
 similarity = WatermarkEvaluator.text_similarity("Hello", "12345")
 self.assertEqual(similarity, 0.0)

def run_basic_tests():
 """运行基本功能测试"""
 print("开始运行基本功能测试...")

 # 创建测试套件
 loader = unittest.TestLoader()
 suite = unittest.TestSuite()

 # 添加测试类
 suite.addTests(loader.loadTestsFromTestCase(TestWatermarkSystem))
 suite.addTests(loader.loadTestsFromTestCase(TestQualityMetrics))
 suite.addTests(loader.loadTestsFromTestCase(TestWatermarkEvaluator))

 # 运行测试
 runner = unittest.TextTestRunner(verbosity=2)
 result = runner.run(suite)

 # 打印结果摘要
 print(f"\n测试结果摘要:")
 print(f"运行测试: {result.testsRun}")
 print(f"失败: {len(result.failures)}")
 print(f"错误: {len(result.errors)}")

 if result.failures:
 print("\n失败的测试:")
 for test, traceback in result.failures:
 print(f"- {test}: {traceback}")

 if result.errors:
 print("\n错误的测试:")
 for test, traceback in result.errors:
 print(f"- {test}: {traceback}")

 return result.wasSuccessful()

if __name__ == "__main__":
 success = run_basic_tests()
 exit(0 if success else 1)
