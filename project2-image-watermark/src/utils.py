"""
工具函数模块

提供图像处理、质量评估等辅助功能
"""

import cv2
import numpy as np
import os
from typing import Tuple, List
import matplotlib.pyplot as plt


class ImageUtils:
    """图像处理工具类"""
    
    @staticmethod
    def load_image(image_path: str, grayscale: bool = True) -> np.ndarray:
        """加载图像"""
        if grayscale:
            image = cv2.imread(image_path, cv2.IMREAD_GRAYSCALE)
        else:
            image = cv2.imread(image_path, cv2.IMREAD_COLOR)
        
        if image is None:
            raise ValueError(f"无法加载图像: {image_path}")
        return image
    
    @staticmethod
    def save_image(image: np.ndarray, output_path: str) -> None:
        """保存图像"""
        os.makedirs(os.path.dirname(output_path), exist_ok=True)
        cv2.imwrite(output_path, image)
    
    @staticmethod
    def ensure_even_dimensions(image: np.ndarray, block_size: int = 8) -> np.ndarray:
        """确保图像尺寸是块大小的倍数"""
        height, width = image.shape[:2]
        new_height = (height // block_size) * block_size
        new_width = (width // block_size) * block_size
        return image[:new_height, :new_width]
    
    @staticmethod
    def create_test_image(width: int = 512, height: int = 512) -> np.ndarray:
        """创建测试图像"""
        # 创建基础噪声图像
        image = np.random.randint(0, 256, (height, width), dtype=np.uint8)
        
        # 添加一些结构化内容
        cv2.rectangle(image, (50, 50), (width-50, height-50), 128, 2)
        cv2.circle(image, (width//2, height//2), min(width, height)//4, 200, -1)
        cv2.putText(image, 'TEST', (width//4, height//2), 
                   cv2.FONT_HERSHEY_SIMPLEX, 2, 0, 3)
        
        return image


class QualityMetrics:
    """图像质量评估工具类"""
    
    @staticmethod
    def calculate_psnr(original: np.ndarray, processed: np.ndarray) -> float:
        """计算峰值信噪比(PSNR)"""
        # 确保图像尺寸一致
        min_height = min(original.shape[0], processed.shape[0])
        min_width = min(original.shape[1], processed.shape[1])
        
        original = original[:min_height, :min_width]
        processed = processed[:min_height, :min_width]
        
        mse = np.mean((original.astype(np.float32) - processed.astype(np.float32)) ** 2)
        
        if mse == 0:
            return float('inf')
        
        max_pixel = 255.0
        psnr = 20 * np.log10(max_pixel / np.sqrt(mse))
        return psnr
    
    @staticmethod
    def calculate_ssim(original: np.ndarray, processed: np.ndarray) -> float:
        """计算结构相似性指数(SSIM)"""
        # 简化的SSIM计算
        original = original.astype(np.float32)
        processed = processed.astype(np.float32)
        
        # 确保尺寸一致
        min_height = min(original.shape[0], processed.shape[0])
        min_width = min(original.shape[1], processed.shape[1])
        
        original = original[:min_height, :min_width]
        processed = processed[:min_height, :min_width]
        
        # 计算均值
        mu1 = np.mean(original)
        mu2 = np.mean(processed)
        
        # 计算方差和协方差
        sigma1_sq = np.var(original)
        sigma2_sq = np.var(processed)
        sigma12 = np.mean((original - mu1) * (processed - mu2))
        
        # SSIM常数
        c1 = (0.01 * 255) ** 2
        c2 = (0.03 * 255) ** 2
        
        # 计算SSIM
        ssim = ((2 * mu1 * mu2 + c1) * (2 * sigma12 + c2)) / \
               ((mu1**2 + mu2**2 + c1) * (sigma1_sq + sigma2_sq + c2))
        
        return ssim
    
    @staticmethod
    def calculate_mse(original: np.ndarray, processed: np.ndarray) -> float:
        """计算均方误差(MSE)"""
        min_height = min(original.shape[0], processed.shape[0])
        min_width = min(original.shape[1], processed.shape[1])
        
        original = original[:min_height, :min_width]
        processed = processed[:min_height, :min_width]
        
        mse = np.mean((original.astype(np.float32) - processed.astype(np.float32)) ** 2)
        return mse


class WatermarkEvaluator:
    """水印评估工具类"""
    
    @staticmethod
    def calculate_extraction_accuracy(original_text: str, extracted_text: str) -> float:
        """计算水印提取准确率"""
        if not original_text:
            return 0.0
        
        # 字符级准确率
        if not extracted_text:
            return 0.0
        
        original_chars = set(original_text.lower())
        extracted_chars = set(extracted_text.lower())
        
        if not original_chars:
            return 0.0
        
        # 计算交集占原始文本的比例
        common_chars = original_chars & extracted_chars
        accuracy = len(common_chars) / len(original_chars)
        
        return accuracy
    
    @staticmethod
    def calculate_bit_error_rate(original_bits: str, extracted_bits: str) -> float:
        """计算比特错误率(BER)"""
        if len(original_bits) != len(extracted_bits):
            # 调整长度到较短的那个
            min_length = min(len(original_bits), len(extracted_bits))
            original_bits = original_bits[:min_length]
            extracted_bits = extracted_bits[:min_length]
        
        if not original_bits:
            return 1.0
        
        errors = sum(o != e for o, e in zip(original_bits, extracted_bits))
        ber = errors / len(original_bits)
        return ber
    
    @staticmethod
    def text_similarity(text1: str, text2: str) -> float:
        """计算文本相似度（简化版）"""
        if not text1 or not text2:
            return 0.0
        
        # 转换为小写并去除空格
        text1 = text1.lower().replace(' ', '')
        text2 = text2.lower().replace(' ', '')
        
        # 计算最长公共子序列长度
        m, n = len(text1), len(text2)
        dp = [[0] * (n + 1) for _ in range(m + 1)]
        
        for i in range(1, m + 1):
            for j in range(1, n + 1):
                if text1[i-1] == text2[j-1]:
                    dp[i][j] = dp[i-1][j-1] + 1
                else:
                    dp[i][j] = max(dp[i-1][j], dp[i][j-1])
        
        lcs_length = dp[m][n]
        similarity = (2 * lcs_length) / (m + n) if (m + n) > 0 else 0
        return similarity


class Visualizer:
    """可视化工具类"""
    
    @staticmethod
    def plot_image_comparison(original: np.ndarray, processed: np.ndarray, 
                            title1: str = "Original", title2: str = "Processed",
                            save_path: str = None) -> None:
        """绘制图像对比"""
        plt.figure(figsize=(12, 5))
        
        plt.subplot(1, 2, 1)
        plt.imshow(original, cmap='gray')
        plt.title(title1)
        plt.axis('off')
        
        plt.subplot(1, 2, 2)
        plt.imshow(processed, cmap='gray')
        plt.title(title2)
        plt.axis('off')
        
        plt.tight_layout()
        
        if save_path:
            plt.savefig(save_path, dpi=150, bbox_inches='tight')
        else:
            plt.show()
        
        plt.close()
    
    @staticmethod
    def plot_attack_results(results_dict: dict, save_path: str = None) -> None:
        """绘制攻击测试结果"""
        attack_names = list(results_dict.keys())
        success_rates = [results_dict[name].get('accuracy', 0) for name in attack_names]
        
        plt.figure(figsize=(15, 8))
        bars = plt.bar(range(len(attack_names)), success_rates)
        plt.xlabel('Attack Types')
        plt.ylabel('Extraction Accuracy')
        plt.title('Watermark Robustness Test Results')
        plt.xticks(range(len(attack_names)), attack_names, rotation=45, ha='right')
        plt.ylim(0, 1.0)
        
        # 添加数值标签
        for bar, acc in zip(bars, success_rates):
            plt.text(bar.get_x() + bar.get_width()/2, bar.get_height() + 0.01,
                    f'{acc:.2f}', ha='center', va='bottom')
        
        plt.tight_layout()
        
        if save_path:
            plt.savefig(save_path, dpi=150, bbox_inches='tight')
        else:
            plt.show()
        
        plt.close()


def main():
    """测试工具函数"""
    print("测试图像工具...")
    
    # 创建测试图像
    test_img = ImageUtils.create_test_image(256, 256)
    
    # 保存测试图像
    os.makedirs("samples", exist_ok=True)
    ImageUtils.save_image(test_img, "samples/test_utils.png")
    
    # 测试质量评估
    noisy_img = test_img + np.random.normal(0, 10, test_img.shape).astype(np.uint8)
    
    metrics = QualityMetrics()
    psnr = metrics.calculate_psnr(test_img, noisy_img)
    ssim = metrics.calculate_ssim(test_img, noisy_img)
    mse = metrics.calculate_mse(test_img, noisy_img)
    
    print(f"PSNR: {psnr:.2f} dB")
    print(f"SSIM: {ssim:.4f}")
    print(f"MSE: {mse:.2f}")
    
    # 测试水印评估
    evaluator = WatermarkEvaluator()
    original_text = "Copyright 2025"
    extracted_text = "Copyright 2025"
    
    accuracy = evaluator.calculate_extraction_accuracy(original_text, extracted_text)
    similarity = evaluator.text_similarity(original_text, extracted_text)
    
    print(f"提取准确率: {accuracy:.2%}")
    print(f"文本相似度: {similarity:.2%}")
    
    print("工具测试完成！")


if __name__ == "__main__":
    main()
