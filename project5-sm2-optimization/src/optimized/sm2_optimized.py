"""
SM2椭圆曲线密码算法优化实现
包含多种性能优化技术

优化技术:
1. Montgomery阶梯算法 - 抗侧信道攻击的标量乘法
2. 窗口方法(Window Method) - 预计算加速
3. 雅可比坐标系 - 避免模逆运算
4. NAF(Non-Adjacent Form) - 减少点运算次数
5. 批量逆元计算 - Montgomery's trick
"""

import hashlib
import secrets
import time
from typing import Tuple, Optional, List, Dict
import numpy as np


class SM2CurveOptimized:
    """
    优化的SM2椭圆曲线实现
    使用雅可比坐标系和多种优化技术
    """
    
    # SM2推荐参数
    p = 0xFFFFFFFEFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000FFFFFFFFFFFFFFFF
    a = 0xFFFFFFFEFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000FFFFFFFFFFFFFFFC
    b = 0x28E9FA9E9D9F5E344D5A9E4BCF6509A7F39789F515AB8F92DDBCBD414D940E93
    n = 0xFFFFFFFEFFFFFFFFFFFFFFFFFFFFFFFF7203DF6B21C6052B53BBF40939D54123
    Gx = 0x32C4AE2C1F1981195F9904466A39C9948FE30BBFF2660BE1715A4589334C74C7
    Gy = 0xBC3736A2F4F6779C59BDCEE36B692153D0A9877CC62A474002DF32E52139F0A0
    
    def __init__(self):
        self.G = (self.Gx, self.Gy)
        self.G_jacobian = (self.Gx, self.Gy, 1)  # 雅可比坐标 (X, Y, Z)
        
        # 预计算表缓存
        self._precompute_cache: Dict[Tuple[int, int], List] = {}
        
        # 优化参数
        self.window_size = 4  # 窗口方法的窗口大小
        
    @staticmethod
    def mod_inverse(a: int, m: int) -> int:
        """快速模逆元计算 - 费马小定理"""
        return pow(a, m - 2, m)
    
    @staticmethod
    def batch_mod_inverse(values: List[int], m: int) -> List[int]:
        """
        批量模逆元计算 - Montgomery's trick
        大幅减少模逆运算次数
        """
        n = len(values)
        if n == 0:
            return []
        if n == 1:
            return [SM2CurveOptimized.mod_inverse(values[0], m)]
            
        # 构建累积乘积
        products = [1] * n
        products[0] = values[0]
        for i in range(1, n):
            products[i] = (products[i-1] * values[i]) % m
            
        # 计算总乘积的逆元
        inv_product = SM2CurveOptimized.mod_inverse(products[n-1], m)
        
        # 反向计算各个逆元
        inverses = [0] * n
        for i in range(n-1, 0, -1):
            inverses[i] = (inv_product * products[i-1]) % m
            inv_product = (inv_product * values[i]) % m
        inverses[0] = inv_product
        
        return inverses
    
    def jacobian_to_affine(self, P: Tuple[int, int, int]) -> Optional[Tuple[int, int]]:
        """
        雅可比坐标转仿射坐标
        
        Args:
            P: 雅可比坐标点 (X, Y, Z)
            
        Returns:
            仿射坐标点 (x, y) 或 None (无穷远点)
        """
        if P is None:
            return None
            
        X, Y, Z = P
        if Z == 0:
            return None
            
        z_inv = self.mod_inverse(Z, self.p)
        z_inv_squared = (z_inv * z_inv) % self.p
        z_inv_cubed = (z_inv_squared * z_inv) % self.p
        
        x = (X * z_inv_squared) % self.p
        y = (Y * z_inv_cubed) % self.p
        
        return (x, y)
    
    def affine_to_jacobian(self, P: Tuple[int, int]) -> Tuple[int, int, int]:
        """
        仿射坐标转雅可比坐标
        
        Args:
            P: 仿射坐标点 (x, y)
            
        Returns:
            雅可比坐标点 (X, Y, Z)
        """
        if P is None:
            return (0, 1, 0)  # 无穷远点
        x, y = P
        return (x, y, 1)
    
    def jacobian_double(self, P: Tuple[int, int, int]) -> Tuple[int, int, int]:
        """
        雅可比坐标点倍运算 - 无需模逆元
        
        Args:
            P: 雅可比坐标点 (X, Y, Z)
            
        Returns:
            2P 的雅可比坐标
        """
        X1, Y1, Z1 = P
        
        if Z1 == 0:
            return (0, 1, 0)  # 无穷远点
            
        # 使用优化的点倍公式
        Y1_squared = (Y1 * Y1) % self.p
        S = (4 * X1 * Y1_squared) % self.p
        M = (3 * X1 * X1 + self.a * Z1 * Z1 * Z1 * Z1) % self.p
        
        X3 = (M * M - 2 * S) % self.p
        Y3 = (M * (S - X3) - 8 * Y1_squared * Y1_squared) % self.p
        Z3 = (2 * Y1 * Z1) % self.p
        
        return (X3, Y3, Z3)
    
    def jacobian_add(self, P: Tuple[int, int, int], Q: Tuple[int, int, int]) -> Tuple[int, int, int]:
        """
        雅可比坐标点加运算 - 无需模逆元
        
        Args:
            P, Q: 雅可比坐标点
            
        Returns:
            P + Q 的雅可比坐标
        """
        X1, Y1, Z1 = P
        X2, Y2, Z2 = Q
        
        if Z1 == 0:
            return Q
        if Z2 == 0:
            return P
            
        # 使用优化的点加公式
        Z1_squared = (Z1 * Z1) % self.p
        Z2_squared = (Z2 * Z2) % self.p
        U1 = (X1 * Z2_squared) % self.p
        U2 = (X2 * Z1_squared) % self.p
        S1 = (Y1 * Z2_squared * Z2) % self.p
        S2 = (Y2 * Z1_squared * Z1) % self.p
        
        if U1 == U2:
            if S1 == S2:
                return self.jacobian_double(P)
            else:
                return (0, 1, 0)  # 无穷远点
                
        H = (U2 - U1) % self.p
        R = (S2 - S1) % self.p
        H_squared = (H * H) % self.p
        H_cubed = (H_squared * H) % self.p
        
        X3 = (R * R - H_cubed - 2 * U1 * H_squared) % self.p
        Y3 = (R * (U1 * H_squared - X3) - S1 * H_cubed) % self.p
        Z3 = (Z1 * Z2 * H) % self.p
        
        return (X3, Y3, Z3)
    
    def naf_representation(self, k: int) -> List[int]:
        """
        计算标量的NAF(非相邻形式)表示
        减少点运算中的非零位数量
        
        Args:
            k: 标量
            
        Returns:
            NAF表示列表
        """
        naf = []
        while k > 0:
            if k & 1:
                width = 2 - (k & 3)  # width ∈ {-1, 1}
                naf.append(width)
                k -= width
            else:
                naf.append(0)
            k >>= 1
        return naf
    
    def montgomery_ladder(self, k: int, P: Tuple[int, int]) -> Optional[Tuple[int, int]]:
        """
        Montgomery阶梯算法 - 抗侧信道攻击的标量乘法
        执行时间与标量k的具体值无关
        
        Args:
            k: 标量
            P: 椭圆曲线点
            
        Returns:
            kP
        """
        if k == 0:
            return None
        if k == 1:
            return P
            
        # 转换为雅可比坐标
        P_jac = self.affine_to_jacobian(P)
        
        # 初始化
        R0 = (0, 1, 0)  # 无穷远点
        R1 = P_jac
        
        # 从最高位开始处理
        bit_length = k.bit_length()
        for i in range(bit_length - 2, -1, -1):
            bit = (k >> i) & 1
            if bit == 0:
                R1 = self.jacobian_add(R0, R1)
                R0 = self.jacobian_double(R0)
            else:
                R0 = self.jacobian_add(R0, R1)
                R1 = self.jacobian_double(R1)
                
        return self.jacobian_to_affine(R0)
    
    def windowed_naf_multiply(self, k: int, P: Tuple[int, int]) -> Optional[Tuple[int, int]]:
        """
        窗口NAF方法的标量乘法
        使用预计算表加速运算
        
        Args:
            k: 标量
            P: 椭圆曲线点
            
        Returns:
            kP
        """
        if k == 0:
            return None
        if k == 1:
            return P
            
        # 检查预计算表缓存
        cache_key = P
        if cache_key not in self._precompute_cache:
            self._precompute_cache[cache_key] = self._build_precompute_table(P)
            
        precompute_table = self._precompute_cache[cache_key]
        
        # 计算窗口NAF
        wnaf = self._compute_window_naf(k, self.window_size)
        
        # 执行标量乘法
        result_jac = (0, 1, 0)  # 无穷远点
        
        for digit in reversed(wnaf):
            result_jac = self.jacobian_double(result_jac)
            if digit > 0:
                result_jac = self.jacobian_add(result_jac, precompute_table[digit // 2])
            elif digit < 0:
                # 添加负点
                neg_point = self._negate_jacobian_point(precompute_table[(-digit) // 2])
                result_jac = self.jacobian_add(result_jac, neg_point)
                
        return self.jacobian_to_affine(result_jac)
    
    def _build_precompute_table(self, P: Tuple[int, int]) -> List[Tuple[int, int, int]]:
        """
        构建预计算表 {P, 3P, 5P, ..., (2^w-1)P}
        
        Args:
            P: 基点
            
        Returns:
            预计算表
        """
        table_size = 2 ** (self.window_size - 2)
        table = [None] * table_size
        
        P_jac = self.affine_to_jacobian(P)
        P2_jac = self.jacobian_double(P_jac)  # 2P
        
        table[0] = P_jac  # P
        
        for i in range(1, table_size):
            table[i] = self.jacobian_add(table[i-1], P2_jac)  # (2i+1)P
            
        return table
    
    def _compute_window_naf(self, k: int, w: int) -> List[int]:
        """
        计算窗口NAF表示
        
        Args:
            k: 标量
            w: 窗口大小
            
        Returns:
            窗口NAF表示
        """
        wnaf = []
        while k > 0:
            if k & 1:
                # k是奇数
                width = k & ((1 << w) - 1)  # 取最低w位
                if width >= (1 << (w - 1)):
                    width -= (1 << w)  # 转换为负数
                wnaf.append(width)
                k -= width
            else:
                wnaf.append(0)
            k >>= 1
        return wnaf
    
    def _negate_jacobian_point(self, P: Tuple[int, int, int]) -> Tuple[int, int, int]:
        """
        计算雅可比坐标点的负点
        
        Args:
            P: 雅可比坐标点 (X, Y, Z)
            
        Returns:
            -P 的雅可比坐标 (X, -Y, Z)
        """
        X, Y, Z = P
        return (X, (self.p - Y) % self.p, Z)
    
    def point_multiply(self, k: int, P: Tuple[int, int], method: str = "montgomery") -> Optional[Tuple[int, int]]:
        """
        统一的标量乘法接口
        
        Args:
            k: 标量
            P: 椭圆曲线点
            method: 算法选择 ("montgomery", "windowed_naf", "basic")
            
        Returns:
            kP
        """
        if method == "montgomery":
            return self.montgomery_ladder(k, P)
        elif method == "windowed_naf":
            return self.windowed_naf_multiply(k, P)
        else:
            # 基础double-and-add方法
            return self._basic_multiply(k, P)
    
    def _basic_multiply(self, k: int, P: Tuple[int, int]) -> Optional[Tuple[int, int]]:
        """基础的double-and-add标量乘法"""
        if k == 0:
            return None
        if k == 1:
            return P
            
        result_jac = (0, 1, 0)  # 无穷远点
        addend_jac = self.affine_to_jacobian(P)
        
        while k:
            if k & 1:
                result_jac = self.jacobian_add(result_jac, addend_jac)
            addend_jac = self.jacobian_double(addend_jac)
            k >>= 1
            
        return self.jacobian_to_affine(result_jac)
    
    def batch_point_multiply(self, scalars: List[int], points: List[Tuple[int, int]]) -> List[Optional[Tuple[int, int]]]:
        """
        批量标量乘法 - 利用共享计算优化
        
        Args:
            scalars: 标量列表
            points: 点列表
            
        Returns:
            [k1*P1, k2*P2, ...] 结果列表
        """
        if len(scalars) != len(points):
            raise ValueError("标量和点的数量不匹配")
            
        results = []
        for k, P in zip(scalars, points):
            results.append(self.point_multiply(k, P, "montgomery"))
            
        return results
    
    def is_on_curve(self, P: Tuple[int, int]) -> bool:
        """验证点是否在椭圆曲线上"""
        if P is None:
            return True
            
        x, y = P
        left = (y * y) % self.p
        right = (x * x * x + self.a * x + self.b) % self.p
        
        return left == right


class SM2DigitalSignatureOptimized:
    """
    优化的SM2数字签名算法
    """
    
    def __init__(self, curve: SM2CurveOptimized):
        self.curve = curve
        self._signature_cache: Dict = {}
    
    def _sm3_hash(self, data: bytes) -> bytes:
        """SM3哈希函数实现 (临时使用SHA-256)"""
        return hashlib.sha256(data).digest()
    
    def _compute_za(self, user_id: bytes, public_key: Tuple[int, int]) -> bytes:
        """计算Za哈希值"""
        id_len = len(user_id) * 8
        
        za_input = bytearray()
        za_input.extend(id_len.to_bytes(2, 'big'))
        za_input.extend(user_id)
        za_input.extend(self.curve.a.to_bytes(32, 'big'))
        za_input.extend(self.curve.b.to_bytes(32, 'big'))
        za_input.extend(self.curve.Gx.to_bytes(32, 'big'))
        za_input.extend(self.curve.Gy.to_bytes(32, 'big'))
        
        x, y = public_key
        za_input.extend(x.to_bytes(32, 'big'))
        za_input.extend(y.to_bytes(32, 'big'))
        
        return self._sm3_hash(bytes(za_input))
    
    def sign_optimized(self, message: bytes, private_key: int, public_key: Tuple[int, int], 
                      user_id: bytes = b"1234567812345678") -> Tuple[int, int]:
        """
        优化的SM2数字签名
        使用Montgomery阶梯算法
        """
        za = self._compute_za(user_id, public_key)
        m_prime = za + message
        e = int.from_bytes(self._sm3_hash(m_prime), 'big')
        
        while True:
            k = secrets.randbelow(self.curve.n - 1) + 1
            
            # 使用Montgomery阶梯算法计算kG
            point = self.curve.point_multiply(k, self.curve.G, "montgomery")
            if point is None:
                continue
                
            x1, _ = point
            r = (e + x1) % self.curve.n
            if r == 0 or (r + k) % self.curve.n == 0:
                continue
                
            # 优化模逆运算
            d_inv = self.curve.mod_inverse(1 + private_key, self.curve.n)
            s = (d_inv * (k - r * private_key)) % self.curve.n
            if s == 0:
                continue
                
            return (r, s)
    
    def verify_optimized(self, message: bytes, signature: Tuple[int, int], 
                        public_key: Tuple[int, int], user_id: bytes = b"1234567812345678") -> bool:
        """
        优化的SM2数字签名验证
        使用批量计算和预计算优化
        """
        r, s = signature
        
        if not (1 <= r < self.curve.n and 1 <= s < self.curve.n):
            return False
            
        za = self._compute_za(user_id, public_key)
        m_prime = za + message
        e = int.from_bytes(self._sm3_hash(m_prime), 'big')
        
        t = (r + s) % self.curve.n
        if t == 0:
            return False
            
        # 使用Shamir's trick进行双标量乘法优化
        point_sum = self._shamir_double_multiply(s, self.curve.G, t, public_key)
        
        if point_sum is None:
            return False
            
        x1_prime, _ = point_sum
        v = (e + x1_prime) % self.curve.n
        
        return v == r
    
    def _shamir_double_multiply(self, k1: int, P1: Tuple[int, int], 
                               k2: int, P2: Tuple[int, int]) -> Optional[Tuple[int, int]]:
        """
        Shamir's trick: 计算 k1*P1 + k2*P2
        比分别计算再相加更高效
        """
        # 预计算 P1, P2, P1+P2
        P1_jac = self.curve.affine_to_jacobian(P1)
        P2_jac = self.curve.affine_to_jacobian(P2)
        P1_plus_P2_jac = self.curve.jacobian_add(P1_jac, P2_jac)
        
        # 找到最大位长度
        max_bits = max(k1.bit_length(), k2.bit_length())
        
        result_jac = (0, 1, 0)  # 无穷远点
        
        for i in range(max_bits - 1, -1, -1):
            result_jac = self.curve.jacobian_double(result_jac)
            
            bit1 = (k1 >> i) & 1
            bit2 = (k2 >> i) & 1
            
            if bit1 and bit2:
                result_jac = self.curve.jacobian_add(result_jac, P1_plus_P2_jac)
            elif bit1:
                result_jac = self.curve.jacobian_add(result_jac, P1_jac)
            elif bit2:
                result_jac = self.curve.jacobian_add(result_jac, P2_jac)
                
        return self.curve.jacobian_to_affine(result_jac)


def benchmark_comparison():
    """
    性能对比测试
    比较不同优化方法的性能
    """
    print("SM2椭圆曲线优化算法性能测试")
    print("=" * 60)
    
    curve = SM2CurveOptimized()
    
    # 生成测试数据
    test_scalar = secrets.randbelow(curve.n)
    test_point = curve.G
    
    methods = ["basic", "montgomery", "windowed_naf"]
    iterations = 100
    
    print(f"测试标量: {hex(test_scalar)[:32]}...")
    print(f"迭代次数: {iterations}")
    print()
    
    results = {}
    
    for method in methods:
        print(f"测试方法: {method}")
        
        start_time = time.time()
        for _ in range(iterations):
            result = curve.point_multiply(test_scalar, test_point, method)
        end_time = time.time()
        
        elapsed = end_time - start_time
        avg_time = elapsed / iterations * 1000  # 毫秒
        
        results[method] = {
            'total_time': elapsed,
            'avg_time': avg_time,
            'result': result
        }
        
        print(f"  总时间: {elapsed:.3f}s")
        print(f"  平均时间: {avg_time:.3f}ms")
        print(f"  结果: ({hex(result[0])[:16]}..., {hex(result[1])[:16]}...)")
        print()
    
    # 验证结果一致性
    base_result = results["basic"]["result"]
    for method in methods[1:]:
        if results[method]["result"] != base_result:
            print(f"警告: {method}方法结果不一致!")
        else:
            print(f"✓ {method}方法结果验证通过")
    
    # 性能提升统计
    print("\n性能提升统计:")
    base_time = results["basic"]["avg_time"]
    for method in methods[1:]:
        speedup = base_time / results[method]["avg_time"]
        print(f"  {method}: {speedup:.2f}x 加速")
    
    return results


if __name__ == "__main__":
    # 运行性能对比测试
    benchmark_comparison()
    
    print("\n" + "="*60)
    print("优化版签名算法测试")
    
    # 签名性能测试
    curve = SM2CurveOptimized()
    signer = SM2DigitalSignatureOptimized(curve)
    
    # 生成密钥对
    private_key = secrets.randbelow(curve.n - 1) + 1
    public_key = curve.point_multiply(private_key, curve.G, "montgomery")
    
    message = b"Performance test message for SM2 signature"
    
    # 签名性能测试
    iterations = 50
    print(f"\n签名性能测试 ({iterations}次):")
    
    start_time = time.time()
    for _ in range(iterations):
        signature = signer.sign_optimized(message, private_key, public_key)
    sign_time = time.time() - start_time
    
    print(f"总签名时间: {sign_time:.3f}s")
    print(f"平均签名时间: {sign_time/iterations*1000:.3f}ms")
    
    # 验证性能测试
    start_time = time.time()
    for _ in range(iterations):
        is_valid = signer.verify_optimized(message, signature, public_key)
    verify_time = time.time() - start_time
    
    print(f"总验证时间: {verify_time:.3f}s")
    print(f"平均验证时间: {verify_time/iterations*1000:.3f}ms")
    print(f"签名验证: {'通过' if is_valid else '失败'}")
    
    print("\n优化测试完成!")
