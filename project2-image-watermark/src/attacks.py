"""
图像攻击测试模块

实现各种图像攻击方法，用于测试水印的鲁棒性
包括几何变换、图像处理、压缩等攻击方式
"""

import cv2
import numpy as np
from PIL import Image, ImageEnhance, ImageFilter
from typing import Tuple, List
import os

class ImageAttacks:
 """图像攻击测试类"""

 @staticmethod
 def horizontal_flip(image: np.ndarray) -> np.ndarray:
 """水平翻转攻击"""
 return cv2.flip(image, 1)

 @staticmethod
 def vertical_flip(image: np.ndarray) -> np.ndarray:
 """垂直翻转攻击"""
 return cv2.flip(image, 0)

 @staticmethod
 def rotation(image: np.ndarray, angle: float) -> np.ndarray:
 """旋转攻击"""
 height, width = image.shape[:2]
 center = (width // 2, height // 2)

 # 获取旋转矩阵
 rotation_matrix = cv2.getRotationMatrix2D(center, angle, 1.0)

 # 执行旋转
 rotated = cv2.warpAffine(image, rotation_matrix, (width, height),
 flags=cv2.INTER_LINEAR, borderMode=cv2.BORDER_REFLECT)
 return rotated

 @staticmethod
 def translation(image: np.ndarray, dx: int, dy: int) -> np.ndarray:
 """平移攻击"""
 height, width = image.shape[:2]

 # 创建平移矩阵
 translation_matrix = np.float32([[1, 0, dx], [0, 1, dy]])

 # 执行平移
 translated = cv2.warpAffine(image, translation_matrix, (width, height),
 borderMode=cv2.BORDER_REFLECT)
 return translated

 @staticmethod
 def crop_attack(image: np.ndarray, crop_ratio: float = 0.8) -> np.ndarray:
 """裁剪攻击"""
 height, width = image.shape[:2]

 # 计算裁剪区域
 new_height = int(height * crop_ratio)
 new_width = int(width * crop_ratio)

 # 计算起始位置（居中裁剪）
 start_y = (height - new_height) // 2
 start_x = (width - new_width) // 2

 # 执行裁剪
 cropped = image[start_y:start_y + new_height, start_x:start_x + new_width]

 # 调整回原始尺寸
 resized = cv2.resize(cropped, (width, height), interpolation=cv2.INTER_LINEAR)
 return resized

 @staticmethod
 def random_crop(image: np.ndarray, crop_ratio: float = 0.7) -> np.ndarray:
 """随机位置裁剪攻击"""
 height, width = image.shape[:2]

 # 计算裁剪区域
 new_height = int(height * crop_ratio)
 new_width = int(width * crop_ratio)

 # 随机起始位置
 max_start_y = height - new_height
 max_start_x = width - new_width
 start_y = np.random.randint(0, max_start_y + 1)
 start_x = np.random.randint(0, max_start_x + 1)

 # 执行裁剪
 cropped = image[start_y:start_y + new_height, start_x:start_x + new_width]

 # 调整回原始尺寸
 resized = cv2.resize(cropped, (width, height), interpolation=cv2.INTER_LINEAR)
 return resized

 @staticmethod
 def scaling(image: np.ndarray, scale_factor: float) -> np.ndarray:
 """缩放攻击"""
 height, width = image.shape[:2]

 # 先缩放到指定比例
 new_height = int(height * scale_factor)
 new_width = int(width * scale_factor)
 scaled = cv2.resize(image, (new_width, new_height), interpolation=cv2.INTER_LINEAR)

 # 再缩放回原始尺寸
 restored = cv2.resize(scaled, (width, height), interpolation=cv2.INTER_LINEAR)
 return restored

 @staticmethod
 def gaussian_blur(image: np.ndarray, kernel_size: int = 5, sigma: float = 1.0) -> np.ndarray:
 """高斯模糊攻击"""
 return cv2.GaussianBlur(image, (kernel_size, kernel_size), sigma)

 @staticmethod
 def gaussian_noise(image: np.ndarray, mean: float = 0, std: float = 10) -> np.ndarray:
 """高斯噪声攻击"""
 noise = np.random.normal(mean, std, image.shape).astype(np.float32)
 noisy_image = image.astype(np.float32) + noise
 return np.clip(noisy_image, 0, 255).astype(np.uint8)

 @staticmethod
 def salt_pepper_noise(image: np.ndarray, noise_ratio: float = 0.01) -> np.ndarray:
 """椒盐噪声攻击"""
 noisy_image = image.copy()
 height, width = image.shape[:2]

 # 添加盐噪声（白点）
 salt_coords = [np.random.randint(0, i - 1, int(noise_ratio * height * width / 2))
 for i in image.shape[:2]]
 noisy_image[salt_coords[0], salt_coords[1]] = 255

 # 添加胡椒噪声（黑点）
 pepper_coords = [np.random.randint(0, i - 1, int(noise_ratio * height * width / 2))
 for i in image.shape[:2]]
 noisy_image[pepper_coords[0], pepper_coords[1]] = 0

 return noisy_image

 @staticmethod
 def brightness_adjustment(image: np.ndarray, factor: float = 1.2) -> np.ndarray:
 """亮度调整攻击"""
 # 使用PIL进行亮度调整
 pil_image = Image.fromarray(image)
 enhancer = ImageEnhance.Brightness(pil_image)
 enhanced = enhancer.enhance(factor)
 return np.array(enhanced)

 @staticmethod
 def contrast_adjustment(image: np.ndarray, factor: float = 1.3) -> np.ndarray:
 """对比度调整攻击"""
 # 使用PIL进行对比度调整
 pil_image = Image.fromarray(image)
 enhancer = ImageEnhance.Contrast(pil_image)
 enhanced = enhancer.enhance(factor)
 return np.array(enhanced)

 @staticmethod
 def jpeg_compression(image: np.ndarray, quality: int = 50) -> np.ndarray:
 """JPEG压缩攻击"""
 # 保存为临时JPEG文件再读取
 temp_path = "temp_compressed.jpg"
 cv2.imwrite(temp_path, image, [cv2.IMWRITE_JPEG_QUALITY, quality])
 compressed = cv2.imread(temp_path, cv2.IMREAD_GRAYSCALE)

 # 清理临时文件
 if os.path.exists(temp_path):
 os.remove(temp_path)

 return compressed

 @staticmethod
 def median_filter(image: np.ndarray, kernel_size: int = 3) -> np.ndarray:
 """中值滤波攻击"""
 return cv2.medianBlur(image, kernel_size)

 @staticmethod
 def histogram_equalization(image: np.ndarray) -> np.ndarray:
 """直方图均衡化攻击"""
 return cv2.equalizeHist(image)

 @staticmethod
 def sharpening(image: np.ndarray) -> np.ndarray:
 """锐化攻击"""
 # 定义锐化卷积核
 kernel = np.array([[-1, -1, -1],
 [-1, 9, -1],
 [-1, -1, -1]], dtype=np.float32)
 sharpened = cv2.filter2D(image.astype(np.float32), -1, kernel)
 return np.clip(sharpened, 0, 255).astype(np.uint8)

class AttackTestSuite:
 """攻击测试套件"""

 def __init__(self):
 self.attacks = ImageAttacks()
 self.test_results = {}

 def run_single_attack(self, image_path: str, attack_name: str,
 output_dir: str = "output/attacks", **kwargs) -> str:
 """运行单个攻击测试"""
 # 读取图像
 image = cv2.imread(image_path, cv2.IMREAD_GRAYSCALE)
 if image is None:
 raise ValueError(f"无法读取图像: {image_path}")

 # 创建输出目录
 os.makedirs(output_dir, exist_ok=True)

 # 执行攻击
 attack_method = getattr(self.attacks, attack_name)
 attacked_image = attack_method(image, **kwargs)

 # 保存攻击后的图像
 output_path = os.path.join(output_dir, f"{attack_name}_attacked.png")
 cv2.imwrite(output_path, attacked_image)

 return output_path

 def run_all_attacks(self, image_path: str, output_dir: str = "output/attacks") -> dict:
 """运行所有攻击测试"""
 attack_configs = {
 'horizontal_flip': {},
 'vertical_flip': {},
 'rotation': {'angle': 15},
 'rotation_45': {'angle': 45},
 'translation': {'dx': 20, 'dy': 15},
 'crop_attack': {'crop_ratio': 0.8},
 'random_crop': {'crop_ratio': 0.7},
 'scaling_down': {'scale_factor': 0.5},
 'scaling_up': {'scale_factor': 1.5},
 'gaussian_blur': {'kernel_size': 5, 'sigma': 1.5},
 'gaussian_noise': {'mean': 0, 'std': 15},
 'salt_pepper_noise': {'noise_ratio': 0.02},
 'brightness_up': {'factor': 1.3},
 'brightness_down': {'factor': 0.7},
 'contrast_up': {'factor': 1.4},
 'contrast_down': {'factor': 0.6},
 'jpeg_compression_high': {'quality': 70},
 'jpeg_compression_low': {'quality': 30},
 'median_filter': {'kernel_size': 3},
 'histogram_equalization': {},
 'sharpening': {}
 }

 results = {}

 for attack_name, params in attack_configs.items():
 try:
 base_attack = attack_name.split('_')[0] if '_' in attack_name else attack_name
 if hasattr(self.attacks, base_attack):
 output_path = self.run_single_attack(
 image_path, base_attack, output_dir, **params
 )
 results[attack_name] = {
 'success': True,
 'output_path': output_path,
 'parameters': params
 }
 print(f" {attack_name} 攻击完成: {output_path}")
 else:
 results[attack_name] = {'success': False, 'error': 'Method not found'}
 except Exception as e:
 results[attack_name] = {'success': False, 'error': str(e)}
 print(f" {attack_name} 攻击失败: {e}")

 return results

def main():
 """测试攻击模块"""
 # 创建测试图像
 test_img = np.random.randint(0, 256, (256, 256), dtype=np.uint8)

 # 添加一些结构化内容
 cv2.rectangle(test_img, (50, 50), (150, 150), 200, -1)
 cv2.circle(test_img, (128, 128), 30, 100, -1)

 os.makedirs("samples", exist_ok=True)
 test_path = "samples/test_for_attacks.png"
 cv2.imwrite(test_path, test_img)

 # 创建攻击测试套件
 test_suite = AttackTestSuite()

 # 运行几个基本攻击测试
 print("开始攻击测试...")

 try:
 # 测试几种基本攻击
 test_suite.run_single_attack(test_path, 'horizontal_flip')
 test_suite.run_single_attack(test_path, 'rotation', angle=30)
 test_suite.run_single_attack(test_path, 'gaussian_blur', kernel_size=7)
 test_suite.run_single_attack(test_path, 'jpeg_compression', quality=50)

 print("基本攻击测试完成！")

 except Exception as e:
 print(f"攻击测试过程中出现错误: {e}")

if __name__ == "__main__":
 main()
