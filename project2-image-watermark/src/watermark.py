"""
数字图像水印系统核心模块

基于DCT变换的频域水印嵌入和提取算法
作者: bbbbhrrrr
日期: 2025-07-12
"""

import cv2
import numpy as np
from PIL import Image
import os
from typing import Tuple, Optional

class WatermarkSystem:
 """数字水印系统主类"""

 def __init__(self, block_size: int = 8, quantization_factor: int = 30):
 """
 初始化水印系统

 Args:
 block_size: DCT块大小，默认8x8
 quantization_factor: 量化因子，控制水印强度
 """
 self.block_size = block_size
 self.quantization_factor = quantization_factor

 def text_to_binary(self, text: str) -> str:
 """将文本转换为二进制字符串"""
 binary = ''.join(format(ord(char), '08b') for char in text)
 # 添加结束标志
 binary += '1111111111111110' # 16位结束标志
 return binary

 def binary_to_text(self, binary: str) -> str:
 """将二进制字符串转换为文本"""
 # 查找结束标志
 end_marker = '1111111111111110'
 if end_marker in binary:
 binary = binary[:binary.find(end_marker)]

 # 确保长度是8的倍数
 while len(binary) % 8 != 0:
 binary = binary[:-1]

 text = ''
 for i in range(0, len(binary), 8):
 byte = binary[i:i+8]
 if len(byte) == 8:
 try:
 char = chr(int(byte, 2))
 if char.isprintable():
 text += char
 except ValueError:
 continue
 return text

 def dct_block(self, block: np.ndarray) -> np.ndarray:
 """对8x8块进行DCT变换"""
 return cv2.dct(block.astype(np.float32))

 def idct_block(self, block: np.ndarray) -> np.ndarray:
 """对8x8块进行IDCT变换"""
 return cv2.idct(block)

 def embed_watermark(self, image_path: str, watermark_text: str,
 output_path: Optional[str] = None, strength: int = None) -> np.ndarray:
 """
 在图像中嵌入水印

 Args:
 image_path: 原始图像路径
 watermark_text: 要嵌入的水印文本
 output_path: 输出图像路径
 strength: 水印强度

 Returns:
 含水印的图像数组
 """
 if strength is not None:
 self.quantization_factor = strength

 # 读取图像
 img = cv2.imread(image_path, cv2.IMREAD_GRAYSCALE)
 if img is None:
 raise ValueError(f"无法读取图像: {image_path}")

 # 确保图像尺寸是块大小的倍数
 height, width = img.shape
 new_height = (height // self.block_size) * self.block_size
 new_width = (width // self.block_size) * self.block_size
 img = img[:new_height, :new_width]

 # 转换水印文本为二进制
 watermark_binary = self.text_to_binary(watermark_text)
 print(f"水印二进制长度: {len(watermark_binary)} bits")

 # 计算可用的嵌入位置数量
 total_blocks = (new_height // self.block_size) * (new_width // self.block_size)
 available_positions = total_blocks * 2 # 每个块嵌入2个bit

 if len(watermark_binary) > available_positions:
 raise ValueError(f"水印过长，需要 {len(watermark_binary)} bits，但只有 {available_positions} 个可用位置")

 # 复制图像用于水印嵌入
 watermarked_img = img.astype(np.float32)

 # 逐块处理
 bit_index = 0
 for i in range(0, new_height, self.block_size):
 for j in range(0, new_width, self.block_size):
 if bit_index >= len(watermark_binary):
 break

 # 提取8x8块
 block = watermarked_img[i:i+self.block_size, j:j+self.block_size]

 # DCT变换
 dct_block = self.dct_block(block)

 # 在中频系数嵌入水印
 # 选择(3,4)和(4,3)位置作为嵌入点
 positions = [(3, 4), (4, 3)]

 for pos_idx, (x, y) in enumerate(positions):
 if bit_index < len(watermark_binary):
 bit = int(watermark_binary[bit_index])

 # 量化索引调制
 coeff = dct_block[x, y]
 quantized = round(coeff / self.quantization_factor)

 # 根据水印bit调整量化值
 if quantized % 2 != bit:
 if quantized % 2 == 0:
 quantized += 1 if bit == 1 else -1
 else:
 quantized += 1 if bit == 0 else -1

 dct_block[x, y] = quantized * self.quantization_factor
 bit_index += 1

 # IDCT变换
 restored_block = self.idct_block(dct_block)
 watermarked_img[i:i+self.block_size, j:j+self.block_size] = restored_block

 # 限制像素值范围
 watermarked_img = np.clip(watermarked_img, 0, 255).astype(np.uint8)

 # 保存图像
 if output_path:
 cv2.imwrite(output_path, watermarked_img)
 print(f"水印图像已保存到: {output_path}")

 return watermarked_img

 def extract_watermark(self, watermarked_image_path: str, max_length: int = 1000) -> str:
 """
 从含水印图像中提取水印

 Args:
 watermarked_image_path: 含水印图像路径
 max_length: 最大提取长度

 Returns:
 提取的水印文本
 """
 # 读取含水印图像
 img = cv2.imread(watermarked_image_path, cv2.IMREAD_GRAYSCALE)
 if img is None:
 raise ValueError(f"无法读取图像: {watermarked_image_path}")

 # 确保图像尺寸是块大小的倍数
 height, width = img.shape
 new_height = (height // self.block_size) * self.block_size
 new_width = (width // self.block_size) * self.block_size
 img = img[:new_height, :new_width].astype(np.float32)

 extracted_bits = ""

 # 逐块处理提取水印
 for i in range(0, new_height, self.block_size):
 for j in range(0, new_width, self.block_size):
 if len(extracted_bits) >= max_length * 8:
 break

 # 提取8x8块
 block = img[i:i+self.block_size, j:j+self.block_size]

 # DCT变换
 dct_block = self.dct_block(block)

 # 从中频系数提取水印
 positions = [(3, 4), (4, 3)]

 for x, y in positions:
 if len(extracted_bits) >= max_length * 8:
 break

 coeff = dct_block[x, y]
 quantized = round(coeff / self.quantization_factor)
 bit = quantized % 2
 extracted_bits += str(bit)

 # 转换为文本
 extracted_text = self.binary_to_text(extracted_bits)
 return extracted_text

 def calculate_psnr(self, original_path: str, watermarked_image: np.ndarray) -> float:
 """计算PSNR值"""
 original = cv2.imread(original_path, cv2.IMREAD_GRAYSCALE)
 if original is None:
 return 0

 # 确保尺寸一致
 h, w = watermarked_image.shape
 original = original[:h, :w]

 mse = np.mean((original.astype(np.float32) - watermarked_image.astype(np.float32)) ** 2)
 if mse == 0:
 return float('inf')

 max_pixel = 255.0
 psnr = 20 * np.log10(max_pixel / np.sqrt(mse))
 return psnr

def main():
 """测试水印系统"""
 # 创建水印系统实例
 watermark_sys = WatermarkSystem(quantization_factor=30)

 # 创建测试图像
 test_img = np.random.randint(0, 256, (512, 512), dtype=np.uint8)
 test_path = "samples/test_image.png"
 os.makedirs("samples", exist_ok=True)
 cv2.imwrite(test_path, test_img)

 try:
 # 嵌入水印
 watermark_text = "Copyright 2025 bbbbhrrrr"
 output_path = "output/watermarked_test.png"
 os.makedirs("output", exist_ok=True)

 watermarked = watermark_sys.embed_watermark(
 image_path=test_path,
 watermark_text=watermark_text,
 output_path=output_path
 )

 # 计算PSNR
 psnr = watermark_sys.calculate_psnr(test_path, watermarked)
 print(f"PSNR: {psnr:.2f} dB")

 # 提取水印
 extracted = watermark_sys.extract_watermark(output_path)
 print(f"原始水印: {watermark_text}")
 print(f"提取水印: {extracted}")

 # 验证准确性
 accuracy = len(set(watermark_text) & set(extracted)) / len(set(watermark_text)) if watermark_text else 0
 print(f"提取准确率: {accuracy:.2%}")

 except Exception as e:
 print(f"测试过程中出现错误: {e}")

if __name__ == "__main__":
 main()
