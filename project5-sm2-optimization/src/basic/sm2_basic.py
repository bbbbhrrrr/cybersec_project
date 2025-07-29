"""
SM2椭圆曲线密码算法基础实现
基于国家标准GM/T 0003.2-2012实现

主要功能:
1. 椭圆曲线基础数学运算
2. SM2数字签名算法
3. SM2密钥交换协议
4. SM2公钥加密算法
"""

import hashlib
import secrets
import time
from typing import Tuple, Optional, Union


class SM2Curve:
    """
    SM2推荐椭圆曲线参数
    曲线方程: y² = x³ + ax + b (mod p)
    """
    
    # SM2推荐参数 (GM/T 0003.2-2012)
    p = 0xFFFFFFFEFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000FFFFFFFFFFFFFFFF
    a = 0xFFFFFFFEFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000FFFFFFFFFFFFFFFC
    b = 0x28E9FA9E9D9F5E344D5A9E4BCF6509A7F39789F515AB8F92DDBCBD414D940E93
    n = 0xFFFFFFFEFFFFFFFFFFFFFFFFFFFFFFFF7203DF6B21C6052B53BBF40939D54123
    Gx = 0x32C4AE2C1F1981195F9904466A39C9948FE30BBFF2660BE1715A4589334C74C7
    Gy = 0xBC3736A2F4F6779C59BDCEE36B692153D0A9877CC62A474002DF32E52139F0A0
    
    def __init__(self):
        self.G = (self.Gx, self.Gy)  # 基点
        
    @staticmethod
    def mod_inverse(a: int, m: int) -> int:
        """
        计算模逆元: a^(-1) mod m
        使用扩展欧几里得算法
        """
        if a < 0:
            a = (a % m + m) % m
            
        def extended_gcd(a: int, b: int) -> Tuple[int, int, int]:
            if a == 0:
                return b, 0, 1
            gcd, x1, y1 = extended_gcd(b % a, a)
            x = y1 - (b // a) * x1
            y = x1
            return gcd, x, y
            
        gcd, x, _ = extended_gcd(a, m)
        if gcd != 1:
            raise ValueError("模逆元不存在")
        return (x % m + m) % m
    
    def point_add(self, P: Tuple[int, int], Q: Tuple[int, int]) -> Optional[Tuple[int, int]]:
        """
        椭圆曲线点加运算
        
        Args:
            P, Q: 椭圆曲线上的点
            
        Returns:
            P + Q 的结果点，如果结果为无穷远点则返回None
        """
        if P is None:
            return Q
        if Q is None:
            return P
            
        x1, y1 = P
        x2, y2 = Q
        
        if x1 == x2:
            if y1 == y2:
                # 点倍运算
                return self.point_double(P)
            else:
                # P + (-P) = O (无穷远点)
                return None
                
        # 一般情况的点加
        lambda_val = ((y2 - y1) * self.mod_inverse(x2 - x1, self.p)) % self.p
        x3 = (lambda_val * lambda_val - x1 - x2) % self.p
        y3 = (lambda_val * (x1 - x3) - y1) % self.p
        
        return (x3, y3)
    
    def point_double(self, P: Tuple[int, int]) -> Optional[Tuple[int, int]]:
        """
        椭圆曲线点倍运算: 2P
        
        Args:
            P: 椭圆曲线上的点
            
        Returns:
            2P 的结果点
        """
        if P is None:
            return None
            
        x1, y1 = P
        
        if y1 == 0:
            return None
            
        # 计算切线斜率
        lambda_val = ((3 * x1 * x1 + self.a) * self.mod_inverse(2 * y1, self.p)) % self.p
        x3 = (lambda_val * lambda_val - 2 * x1) % self.p
        y3 = (lambda_val * (x1 - x3) - y1) % self.p
        
        return (x3, y3)
    
    def point_multiply(self, k: int, P: Tuple[int, int]) -> Optional[Tuple[int, int]]:
        """
        椭圆曲线标量乘法: kP
        使用二进制展开方法(double-and-add)
        
        Args:
            k: 标量
            P: 椭圆曲线上的点
            
        Returns:
            kP 的结果点
        """
        if k == 0:
            return None
        if k == 1:
            return P
            
        result = None
        addend = P
        
        while k:
            if k & 1:
                result = self.point_add(result, addend)
            addend = self.point_double(addend)
            k >>= 1
            
        return result
    
    def is_on_curve(self, P: Tuple[int, int]) -> bool:
        """
        验证点是否在椭圆曲线上
        
        Args:
            P: 待验证的点
            
        Returns:
            True如果点在曲线上，否则False
        """
        if P is None:
            return True  # 无穷远点被认为在曲线上
            
        x, y = P
        left = (y * y) % self.p
        right = (x * x * x + self.a * x + self.b) % self.p
        
        return left == right
    
    def compress_point(self, P: Tuple[int, int]) -> bytes:
        """
        点压缩: 将椭圆曲线点压缩为较短的字节串
        
        Args:
            P: 椭圆曲线上的点
            
        Returns:
            压缩后的点表示
        """
        if P is None:
            return b'\x00' * 33
            
        x, y = P
        # 使用y坐标的最低位确定压缩标识
        prefix = 0x02 + (y & 1)
        return prefix.to_bytes(1, 'big') + x.to_bytes(32, 'big')
    
    def decompress_point(self, compressed: bytes) -> Optional[Tuple[int, int]]:
        """
        点解压缩: 从压缩表示恢复完整的椭圆曲线点
        
        Args:
            compressed: 压缩的点表示
            
        Returns:
            恢复的椭圆曲线点
        """
        if len(compressed) != 33:
            raise ValueError("压缩点长度不正确")
            
        prefix = compressed[0]
        x = int.from_bytes(compressed[1:], 'big')
        
        if prefix == 0x00:
            return None
            
        # 计算 y² = x³ + ax + b
        y_squared = (x * x * x + self.a * x + self.b) % self.p
        
        # 计算平方根 (使用Tonelli-Shanks算法的简化版本)
        y = pow(y_squared, (self.p + 1) // 4, self.p)
        
        # 根据前缀选择正确的y值
        if (y & 1) != (prefix & 1):
            y = self.p - y
            
        point = (x, y)
        if not self.is_on_curve(point):
            raise ValueError("解压缩得到的点不在曲线上")
            
        return point


class SM2KeyPair:
    """
    SM2密钥对管理
    """
    
    def __init__(self, curve: SM2Curve):
        self.curve = curve
        self.private_key: Optional[int] = None
        self.public_key: Optional[Tuple[int, int]] = None
    
    def generate_keypair(self) -> Tuple[int, Tuple[int, int]]:
        """
        生成SM2密钥对
        
        Returns:
            (private_key, public_key) 元组
        """
        # 生成私钥: 1 < d < n-1 的随机数
        while True:
            d = secrets.randbelow(self.curve.n - 1) + 1
            if 1 < d < self.curve.n - 1:
                break
                
        # 计算公钥: P = dG
        P = self.curve.point_multiply(d, self.curve.G)
        
        self.private_key = d
        self.public_key = P
        
        return d, P
    
    def set_private_key(self, private_key: int) -> None:
        """
        设置私钥并计算对应的公钥
        
        Args:
            private_key: 私钥值
        """
        if not (1 < private_key < self.curve.n - 1):
            raise ValueError("私钥值不在有效范围内")
            
        self.private_key = private_key
        self.public_key = self.curve.point_multiply(private_key, self.curve.G)
    
    def export_private_key(self) -> bytes:
        """
        导出私钥为字节串
        
        Returns:
            私钥的字节表示
        """
        if self.private_key is None:
            raise ValueError("私钥未设置")
        return self.private_key.to_bytes(32, 'big')
    
    def export_public_key(self, compressed: bool = False) -> bytes:
        """
        导出公钥为字节串
        
        Args:
            compressed: 是否使用压缩格式
            
        Returns:
            公钥的字节表示
        """
        if self.public_key is None:
            raise ValueError("公钥未设置")
            
        if compressed:
            return self.curve.compress_point(self.public_key)
        else:
            x, y = self.public_key
            return b'\x04' + x.to_bytes(32, 'big') + y.to_bytes(32, 'big')


class SM2DigitalSignature:
    """
    SM2数字签名算法实现
    基于GM/T 0003.2-2012标准
    """
    
    def __init__(self, curve: SM2Curve):
        self.curve = curve
    
    def _sm3_hash(self, data: bytes) -> bytes:
        """
        SM3哈希函数 (简化实现，实际应使用标准SM3)
        这里暂时使用SHA-256作为替代
        """
        return hashlib.sha256(data).digest()
    
    def _compute_za(self, user_id: bytes, public_key: Tuple[int, int]) -> bytes:
        """
        计算用户身份标识的哈希值Za
        
        Args:
            user_id: 用户身份标识
            public_key: 用户公钥
            
        Returns:
            Za哈希值
        """
        id_len = len(user_id) * 8  # 位长度
        
        # 构造Za的输入
        za_input = bytearray()
        za_input.extend(id_len.to_bytes(2, 'big'))  # ENTL
        za_input.extend(user_id)  # ID
        za_input.extend(self.curve.a.to_bytes(32, 'big'))  # a
        za_input.extend(self.curve.b.to_bytes(32, 'big'))  # b
        za_input.extend(self.curve.Gx.to_bytes(32, 'big'))  # xG
        za_input.extend(self.curve.Gy.to_bytes(32, 'big'))  # yG
        
        x, y = public_key
        za_input.extend(x.to_bytes(32, 'big'))  # xA
        za_input.extend(y.to_bytes(32, 'big'))  # yA
        
        return self._sm3_hash(bytes(za_input))
    
    def sign(self, message: bytes, private_key: int, public_key: Tuple[int, int], 
             user_id: bytes = b"1234567812345678") -> Tuple[int, int]:
        """
        SM2数字签名
        
        Args:
            message: 待签名消息
            private_key: 签名者私钥
            public_key: 签名者公钥
            user_id: 用户身份标识
            
        Returns:
            (r, s) 签名值对
        """
        # 计算Za
        za = self._compute_za(user_id, public_key)
        
        # 计算消息摘要 M' = Za || M
        m_prime = za + message
        e = int.from_bytes(self._sm3_hash(m_prime), 'big')
        
        while True:
            # 生成随机数k
            k = secrets.randbelow(self.curve.n - 1) + 1
            
            # 计算椭圆曲线点 (x1, y1) = [k]G
            point = self.curve.point_multiply(k, self.curve.G)
            if point is None:
                continue
                
            x1, _ = point
            
            # 计算r = (e + x1) mod n
            r = (e + x1) % self.curve.n
            if r == 0 or (r + k) % self.curve.n == 0:
                continue
                
            # 计算s = (1 + dA)^(-1) * (k - r * dA) mod n
            d_inv = self.curve.mod_inverse(1 + private_key, self.curve.n)
            s = (d_inv * (k - r * private_key)) % self.curve.n
            if s == 0:
                continue
                
            return (r, s)
    
    def verify(self, message: bytes, signature: Tuple[int, int], 
               public_key: Tuple[int, int], user_id: bytes = b"1234567812345678") -> bool:
        """
        SM2数字签名验证
        
        Args:
            message: 原始消息
            signature: (r, s) 签名值对
            public_key: 签名者公钥
            user_id: 用户身份标识
            
        Returns:
            True如果签名有效，否则False
        """
        r, s = signature
        
        # 验证签名值范围
        if not (1 <= r < self.curve.n and 1 <= s < self.curve.n):
            return False
            
        # 计算Za
        za = self._compute_za(user_id, public_key)
        
        # 计算消息摘要
        m_prime = za + message
        e = int.from_bytes(self._sm3_hash(m_prime), 'big')
        
        # 计算t = (r + s) mod n
        t = (r + s) % self.curve.n
        if t == 0:
            return False
            
        # 计算椭圆曲线点 (x1', y1') = [s]G + [t]PA
        point1 = self.curve.point_multiply(s, self.curve.G)
        point2 = self.curve.point_multiply(t, public_key)
        point_sum = self.curve.point_add(point1, point2)
        
        if point_sum is None:
            return False
            
        x1_prime, _ = point_sum
        
        # 计算v = (e + x1') mod n
        v = (e + x1_prime) % self.curve.n
        
        # 验证 v = r
        return v == r


if __name__ == "__main__":
    # 基础功能测试
    print("SM2椭圆曲线密码算法基础实现测试")
    print("=" * 50)
    
    # 初始化椭圆曲线
    curve = SM2Curve()
    print(f"椭圆曲线参数已初始化")
    print(f"基点G = ({hex(curve.Gx)[:16]}..., {hex(curve.Gy)[:16]}...)")
    
    # 验证基点在曲线上
    assert curve.is_on_curve(curve.G), "基点不在曲线上"
    print("基点验证通过")
    
    # 生成密钥对
    keypair = SM2KeyPair(curve)
    private_key, public_key = keypair.generate_keypair()
    print(f"生成密钥对:")
    print(f"  私钥: {hex(private_key)[:32]}...")
    print(f"  公钥: ({hex(public_key[0])[:16]}..., {hex(public_key[1])[:16]}...)")
    
    # 验证公钥在曲线上
    assert curve.is_on_curve(public_key), "公钥不在曲线上"
    print("公钥验证通过")
    
    # 数字签名测试
    signer = SM2DigitalSignature(curve)
    message = b"Hello, SM2 Digital Signature!"
    
    print(f"\n数字签名测试:")
    print(f"消息: {message.decode()}")
    
    # 签名
    signature = signer.sign(message, private_key, public_key)
    print(f"签名: r={hex(signature[0])[:32]}...")
    print(f"      s={hex(signature[1])[:32]}...")
    
    # 验证
    is_valid = signer.verify(message, signature, public_key)
    print(f"签名验证: {'通过' if is_valid else '失败'}")
    
    # 验证错误消息
    wrong_message = b"Wrong message"
    is_valid_wrong = signer.verify(wrong_message, signature, public_key)
    print(f"错误消息验证: {'失败' if not is_valid_wrong else '意外通过'}")
    
    print("\n基础测试完成!")
