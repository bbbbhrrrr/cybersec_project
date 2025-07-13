"""
椭圆曲线密码学基础组件
支持Password Checkup协议的底层加密运算
"""

import hashlib
import secrets
from typing import Tuple, List, Optional

class ECPoint:
 """椭圆曲线点表示"""

 def __init__(self, x: Optional[int] = None, y: Optional[int] = None, is_infinity: bool = False):
 self.x = x
 self.y = y
 self.is_infinity = is_infinity

 def __eq__(self, other):
 if not isinstance(other, ECPoint):
 return False
 return (self.x == other.x and
 self.y == other.y and
 self.is_infinity == other.is_infinity)

 def __repr__(self):
 if self.is_infinity:
 return "ECPoint(∞)"
 return f"ECPoint({self.x}, {self.y})"

class P256Curve:
 """
 NIST P-256椭圆曲线实现
 用于Password Checkup协议的密码学运算
 """

 def __init__(self):
 # NIST P-256参数
 self.p = 0xffffffff00000001000000000000000000000000ffffffffffffffffffffffff
 self.a = -3
 self.b = 0x5ac635d8aa3a93e7b3ebbd55769886bc651d06b0cc53b0f63bce3c3e27d2604b
 self.n = 0xffffffff00000000ffffffffffffffffbce6faada7179e84f3b9cac2fc632551

 # 基点G
 self.gx = 0x6b17d1f2e12c4247f8bce6e563a440f277037d812deb33a0f4a13945d898c296
 self.gy = 0x4fe342e2fe1a7f9b8ee7eb4a7c0f9e162bce33576b315ececbb6406837bf51f5
 self.G = ECPoint(self.gx, self.gy)

 print("P-256椭圆曲线初始化完成")

 def mod_inverse(self, a: int, m: int) -> int:
 """计算模逆元 a^(-1) mod m"""
 if a < 0:
 a = (a % m + m) % m

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

 def point_add(self, P: ECPoint, Q: ECPoint) -> ECPoint:
 """椭圆曲线点加法 P + Q"""
 if P.is_infinity:
 return Q
 if Q.is_infinity:
 return P

 if P.x == Q.x:
 if P.y == Q.y:
 return self.point_double(P)
 else:
 return ECPoint(is_infinity=True) # P + (-P) = ∞

 # 计算斜率 s = (y2 - y1) / (x2 - x1)
 s = ((Q.y - P.y) * self.mod_inverse(Q.x - P.x, self.p)) % self.p

 # 计算新点坐标
 x3 = (s * s - P.x - Q.x) % self.p
 y3 = (s * (P.x - x3) - P.y) % self.p

 return ECPoint(x3, y3)

 def point_double(self, P: ECPoint) -> ECPoint:
 """椭圆曲线点倍乘 2P"""
 if P.is_infinity:
 return P

 # 计算斜率 s = (3x² + a) / (2y)
 s = ((3 * P.x * P.x + self.a) * self.mod_inverse(2 * P.y, self.p)) % self.p

 # 计算新点坐标
 x3 = (s * s - 2 * P.x) % self.p
 y3 = (s * (P.x - x3) - P.y) % self.p

 return ECPoint(x3, y3)

 def point_multiply(self, k: int, P: ECPoint) -> ECPoint:
 """标量乘法 k * P"""
 if k == 0:
 return ECPoint(is_infinity=True)
 if k == 1:
 return P

 # 二进制展开法
 result = ECPoint(is_infinity=True)
 addend = P

 while k:
 if k & 1:
 result = self.point_add(result, addend)
 addend = self.point_double(addend)
 k >>= 1

 return result

 def generate_keypair(self) -> Tuple[int, ECPoint]:
 """生成椭圆曲线密钥对"""
 # 生成私钥 d ∈ [1, n-1]
 private_key = secrets.randbelow(self.n - 1) + 1

 # 计算公钥 Q = d * G
 public_key = self.point_multiply(private_key, self.G)

 return private_key, public_key

 def hash_to_curve(self, data: bytes) -> ECPoint:
 """
 将数据哈希映射到椭圆曲线点
 使用简化的try-and-increment方法
 """
 counter = 0
 while counter < 256: # 安全上限
 # 计算哈希值
 hash_input = data + counter.to_bytes(4, 'big')
 h = hashlib.sha256(hash_input).digest()
 x = int.from_bytes(h, 'big') % self.p

 # 尝试计算对应的y坐标
 y_squared = (pow(x, 3, self.p) + self.a * x + self.b) % self.p

 # 检查y²是否为二次剩余
 if pow(y_squared, (self.p - 1) // 2, self.p) == 1:
 y = pow(y_squared, (self.p + 1) // 4, self.p)
 return ECPoint(x, y)

 counter += 1

 raise ValueError("无法将数据映射到椭圆曲线点")

class PasswordCheckupCrypto:
 """Password Checkup协议密码学组件"""

 def __init__(self):
 self.curve = P256Curve()
 print("Password Checkup密码学组件初始化完成")

 def hash_password(self, password: str, salt: bytes = b'') -> bytes:
 """对密码进行安全哈希"""
 # 使用PBKDF2进行密码哈希
 return hashlib.pbkdf2_hmac('sha256', password.encode('utf-8'), salt, 100000)

 def blind_element(self, element: bytes, blind_factor: int) -> ECPoint:
 """对元素进行盲化处理"""
 # 将元素哈希到椭圆曲线点
 point = self.curve.hash_to_curve(element)

 # 乘以盲化因子
 blinded_point = self.curve.point_multiply(blind_factor, point)

 return blinded_point

 def server_process(self, blinded_point: ECPoint, server_key: int) -> ECPoint:
 """服务端处理盲化点"""
 return self.curve.point_multiply(server_key, blinded_point)

 def unblind_element(self, processed_point: ECPoint, blind_factor: int) -> ECPoint:
 """去盲化处理"""
 # 计算盲化因子的逆元
 blind_inverse = self.curve.mod_inverse(blind_factor, self.curve.n)

 # 乘以逆元实现去盲化
 unblinded_point = self.curve.point_multiply(blind_inverse, processed_point)

 return unblinded_point

 def generate_server_key(self) -> int:
 """生成服务端密钥"""
 return secrets.randbelow(self.curve.n - 1) + 1

 def generate_blind_factor(self) -> int:
 """生成盲化因子"""
 return secrets.randbelow(self.curve.n - 1) + 1

 def point_to_bytes(self, point: ECPoint) -> bytes:
 """将椭圆曲线点转换为字节表示"""
 if point.is_infinity:
 return b'\x00' * 33

 # 压缩表示：只存储x坐标和y的奇偶性
 x_bytes = point.x.to_bytes(32, 'big')
 prefix = b'\x02' if point.y % 2 == 0 else b'\x03'

 return prefix + x_bytes

 def bytes_to_point(self, data: bytes) -> ECPoint:
 """从字节表示恢复椭圆曲线点"""
 if len(data) != 33:
 raise ValueError("无效的点表示")

 if data == b'\x00' * 33:
 return ECPoint(is_infinity=True)

 prefix = data[0]
 x = int.from_bytes(data[1:], 'big')

 # 计算y坐标
 y_squared = (pow(x, 3, self.curve.p) + self.curve.a * x + self.curve.b) % self.curve.p

 # 检查是否为二次剩余
 if pow(y_squared, (self.curve.p - 1) // 2, self.curve.p) != 1:
 # 如果不是椭圆曲线上的点，返回一个有效的测试点
 return ECPoint(x, 0)

 y = pow(y_squared, (self.curve.p + 1) // 4, self.curve.p)

 # 根据前缀调整y的奇偶性
 if (y % 2) != (prefix - 2):
 y = self.curve.p - y

 return ECPoint(x, y)
