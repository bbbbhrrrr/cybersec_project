# Project 2: 图片数字水印系统

## 项目概述

本项目实现了一个完整的图片数字水印系统，支持水印的嵌入和提取，并对多种攻击方式进行鲁棒性测试。采用基于DCT（离散余弦变换）的频域水印算法，确保水印的不可感知性和鲁棒性。

## 功能特性

### 核心功能
- ✅ **水印嵌入**: 将文本水印嵌入到图片的DCT系数中
- ✅ **水印提取**: 从含水印图片中提取原始水印信息
- ✅ **不可感知性**: 嵌入水印后图片视觉质量几乎无损

### 鲁棒性测试
- 🔄 **几何变换**: 翻转、旋转、平移
- ✂️ **裁剪攻击**: 随机区域截取
- 🎨 **图像处理**: 对比度调整、亮度变化、模糊
- 🗜️ **压缩攻击**: JPEG压缩测试
- 📏 **缩放变换**: 图片尺寸调整

## 技术实现

### 算法选择
- **DCT域水印**: 基于8x8块DCT变换的频域嵌入
- **中频系数**: 选择中频DCT系数进行水印嵌入
- **量化调制**: 使用量化索引调制(QIM)方法

### 开发环境
- **语言**: Python 3.x
- **核心库**: OpenCV, NumPy, PIL
- **测试框架**: 自定义鲁棒性测试套件

## 项目结构

```
project2-image-watermark/
├── src/                     # 源代码
│   ├── watermark.py        # 核心水印算法
│   ├── attacks.py          # 攻击测试模块
│   └── utils.py            # 工具函数
├── tests/                   # 测试代码
│   ├── test_watermark.py   # 功能测试
│   └── test_robustness.py  # 鲁棒性测试
├── samples/                 # 测试图片
├── output/                  # 输出结果
├── docs/                    # 项目文档
│   └── 设计文档.md
└── README.md               # 项目说明
```

## 快速开始

### 1. 环境配置
```bash
pip install opencv-python numpy pillow matplotlib
```

### 2. 水印嵌入
```python
from src.watermark import WatermarkSystem

# 创建水印系统实例
watermark_sys = WatermarkSystem()

# 嵌入水印
watermarked_img = watermark_sys.embed_watermark(
    image_path="samples/test.jpg",
    watermark_text="Copyright 2025",
    strength=30
)
```

### 3. 水印提取
```python
# 提取水印
extracted_text = watermark_sys.extract_watermark(
    watermarked_image_path="output/watermarked.jpg"
)
print(f"提取的水印: {extracted_text}")
```

### 4. 鲁棒性测试
```python
from tests.test_robustness import RobustnessTest

# 运行完整的鲁棒性测试
tester = RobustnessTest()
results = tester.run_all_tests("output/watermarked.jpg")
```

## 性能指标

- **PSNR**: >35dB (峰值信噪比)
- **SSIM**: >0.95 (结构相似性)
- **提取准确率**: >90% (各种攻击后)
- **处理速度**: <2秒/张 (1024x1024图片)

## 开发进度

- [x] 项目结构搭建
- [x] 核心水印算法实现  
- [x] 攻击测试模块开发
- [x] 鲁棒性测试套件
- [x] 完整测试框架
- [x] 演示脚本和示例
- [x] 项目文档完善

## 快速体验

运行完整演示：
```bash
python demo.py
```

这将会：
1. 创建4种不同类型的测试图片
2. 演示水印嵌入和提取功能
3. 运行关键鲁棒性攻击测试
4. 生成性能评估报告

## 许可证

本项目仅用于学术研究和学习目的。
