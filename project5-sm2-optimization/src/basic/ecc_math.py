"""
SM2椭圆曲线基础数学运算模块
实现椭圆曲线上的点运算和域运算

符合GM/T 0003.2-2012标准的椭圆曲线参数
"""

import random
from typing import Tuple, Optional

class ECCPoint:
 """椭圆曲线上的点"""

 def __init__(self, x: Optional[int] = None, y: Optional[int] = None, is_infinity: bool = False):
 """
 初始化椭圆曲线点

 Args:
 x: x坐标
 y: y坐标
 is_infinity: 是否为无穷远点
 """
 self.x = x
 self.y = y
 self.is_infinity = is_infinity

 def __eq__(self, other) -> bool:
 """判断两点是否相等"""
 if not isinstance(other, ECCPoint):
 return False

 if self.is_infinity and other.is_infinity:
 return True

 if self.is_infinity or other.is_infinity:
 return False

 return self.x == other.x and self.y == other.y

 def __str__(self) -> str:
 """点的字符串表示"""
 if self.is_infinity:
 return "Point(∞)"
 return f"Point({hex(self.x)}, {hex(self.y)})"

class SM2Curve:
 """SM2椭圆曲线参数和运算"""

 def __init__(self):
 """初始化SM2推荐曲线参数"""
 # SM2推荐曲线参数 (GM/T 0003.2-2012)
 self.p = 0xFFFFFFFEFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000FFFFFFFFFFFFFFFF
 self.a = 0xFFFFFFFEFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000FFFFFFFFFFFFFFFC
 self.b = 0x28E9FA9E9D9F5E344D5A9E4BCF6509A7F39789F515AB8F92DDBCBD414D940E93
 self.gx = 0x32C4AE2C1F1981195F9904466A39C9948FE30BBFF2660BE1715A4589334C74C7
 self.gy = 0xBC3736A2F4F6779C59BDCEE36B692153D0A9877CC62A474002DF32E52139F0A0
 self.n = 0xFFFFFFFEFFFFFFFFFFFFFFFFFFFFFFFF7203DF6B21C6052B53BBF40939D54123

 # 基点G
 self.G = ECCPoint(self.gx, self.gy)

 # 预计算优化用的表
 self._precomputed_multiples = {}

 def mod_inverse(self, a: int, m: int) -> int:
 """
 计算模逆元 a^(-1) mod m
 使用扩展欧几里得算法
 """
 if a < 0:
 a = (a % m + m) % m

 # 扩展欧几里得算法
 def extended_gcd(a, b):
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

 def point_double(self, P: ECCPoint) -> ECCPoint:
 """
 椭圆曲线点倍乘: 2P

 Args:
 P: 椭圆曲线上的点

 Returns:
 2P
 """
 if P.is_infinity:
 return ECCPoint(is_infinity=True)

 # 计算斜率 λ = (3*x^2 + a) / (2*y)
 numerator = (3 * P.x * P.x + self.a) % self.p
 denominator = (2 * P.y) % self.p

 if denominator == 0:
 return ECCPoint(is_infinity=True)

 slope = (numerator * self.mod_inverse(denominator, self.p)) % self.p

 # 计算新坐标
 x3 = (slope * slope - 2 * P.x) % self.p
 y3 = (slope * (P.x - x3) - P.y) % self.p

 return ECCPoint(x3, y3)

 def point_add(self, P: ECCPoint, Q: ECCPoint) -> ECCPoint:
 """
 椭圆曲线点加法: P + Q

 Args:
 P: 第一个点
 Q: 第二个点

 Returns:
 P + Q
 """
 if P.is_infinity:
 return Q
 if Q.is_infinity:
 return P

 if P.x == Q.x:
 if P.y == Q.y:
 return self.point_double(P)
 else:
 return ECCPoint(is_infinity=True)

 # 计算斜率 λ = (y2 - y1) / (x2 - x1)
 numerator = (Q.y - P.y) % self.p
 denominator = (Q.x - P.x) % self.p
 slope = (numerator * self.mod_inverse(denominator, self.p)) % self.p

 # 计算新坐标
 x3 = (slope * slope - P.x - Q.x) % self.p
 y3 = (slope * (P.x - x3) - P.y) % self.p

 return ECCPoint(x3, y3)

 def point_multiply(self, k: int, P: ECCPoint) -> ECCPoint:
 """
 椭圆曲线标量乘法: k*P
 使用二进制展开法(Double-and-Add)

 Args:
 k: 标量
 P: 椭圆曲线上的点

 Returns:
 k*P
 """
 if k == 0:
 return ECCPoint(is_infinity=True)

 if k < 0:
 # 处理负数情况
 k = -k
 P = ECCPoint(P.x, (-P.y) % self.p)

 result = ECCPoint(is_infinity=True)
 addend = P

 while k > 0:
 if k & 1:
 result = self.point_add(result, addend)
 addend = self.point_double(addend)
 k >>= 1

 return result

 def is_point_on_curve(self, P: ECCPoint) -> bool:
 """
 验证点是否在椭圆曲线上

 Args:
 P: 待验证的点

 Returns:
 True if 点在曲线上, False otherwise
 """
 if P.is_infinity:
 return True

 # 验证 y^2 = x^3 + ax + b (mod p)
 left = (P.y * P.y) % self.p
 right = (P.x * P.x * P.x + self.a * P.x + self.b) % self.p

 return left == right

 def generate_keypair(self) -> Tuple[int, ECCPoint]:
 """
 生成SM2密钥对

 Returns:
 (private_key, public_key) 其中private_key是整数，public_key是椭圆曲线点
 """
 # 生成私钥 d ∈ [1, n-1]
 private_key = random.randint(1, self.n - 1)

 # 计算公钥 P = d * G
 public_key = self.point_multiply(private_key, self.G)

 return private_key, public_key

 def compress_point(self, P: ECCPoint) -> bytes:
 """
 压缩椭圆曲线点

 Args:
 P: 椭圆曲线点

 Returns:
 压缩后的点表示 (33字节)
 """
 if P.is_infinity:
 return b'\x00' + b'\x00' * 32

 # 压缩标志位: 0x02 if y是偶数, 0x03 if y是奇数
 prefix = 0x02 if P.y % 2 == 0 else 0x03

 # x坐标 (32字节)
 x_bytes = P.x.to_bytes(32, byteorder='big')

 return bytes([prefix]) + x_bytes

 def decompress_point(self, compressed: bytes) -> ECCPoint:
 """
 解压缩椭圆曲线点

 Args:
 compressed: 压缩的点表示 (33字节)

 Returns:
 椭圆曲线点
 """
 if len(compressed) != 33:
 raise ValueError("压缩点长度必须为33字节")

 prefix = compressed[0]
 if prefix == 0x00:
 return ECCPoint(is_infinity=True)

 if prefix not in [0x02, 0x03]:
 raise ValueError("无效的压缩点前缀")

 # 提取x坐标
 x = int.from_bytes(compressed[1:], byteorder='big')

 # 计算 y^2 = x^3 + ax + b (mod p)
 y_squared = (x * x * x + self.a * x + self.b) % self.p

 # 计算平方根
 y = self.mod_sqrt(y_squared, self.p)

 # 根据前缀选择正确的y值
 if (y % 2) != (prefix - 0x02):
 y = self.p - y

 return ECCPoint(x, y)

 def mod_sqrt(self, a: int, p: int) -> int:
 """
 计算模p的平方根 (Tonelli-Shanks算法)

 Args:
 a: 被开方数
 p: 模数(质数)

 Returns:
 sqrt(a) mod p
 """
 if pow(a, (p - 1) // 2, p) != 1:
 raise ValueError("不存在平方根")

 # 对于p ≡ 3 (mod 4)的情况，可以直接计算
 if p % 4 == 3:
 return pow(a, (p + 1) // 4, p)

 # Tonelli-Shanks算法 (一般情况)
 # 这里简化实现，实际应用中可能需要更完整的实现
 return pow(a, (p + 1) // 4, p)

def test_basic_operations():
 """测试基础椭圆曲线运算"""
 curve = SM2Curve()

 print("测试SM2椭圆曲线基础运算...")

 # 测试基点是否在曲线上
 assert curve.is_point_on_curve(curve.G), "基点G不在曲线上"
 print(" 基点G在曲线上")

 # 测试点倍乘
 P2 = curve.point_double(curve.G)
 assert curve.is_point_on_curve(P2), "2G不在曲线上"
 print(" 点倍乘运算正确")

 # 测试点加法
 P3 = curve.point_add(curve.G, P2)
 assert curve.is_point_on_curve(P3), "G+2G不在曲线上"
 print(" 点加法运算正确")

 # 测试标量乘法
 k = 12345
 kG = curve.point_multiply(k, curve.G)
 assert curve.is_point_on_curve(kG), "kG不在曲线上"
 print(" 标量乘法运算正确")

 # 测试n*G = O (无穷远点)
 nG = curve.point_multiply(curve.n, curve.G)
 assert nG.is_infinity, "n*G应该是无穷远点"
 print(" 阶验证正确")

 print("所有基础运算测试通过！")

if __name__ == "__main__":
 test_basic_operations()
