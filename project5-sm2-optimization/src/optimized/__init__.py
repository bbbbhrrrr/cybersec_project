"""
SM2椭圆曲线数字签名算法优化实现模块
包含各种优化版本的实现
"""

from .sm2_optimized_v1 import OptimizedSM2Signature, OptimizedSM2Curve

__version__ = "1.0.0"
__author__ = "SM2 Optimization Project"
__description__ = "SM2椭圆曲线数字签名算法优化实现"

__all__ = [
    "OptimizedSM2Signature",
    "OptimizedSM2Curve"
]
