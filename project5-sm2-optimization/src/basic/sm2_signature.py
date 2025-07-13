"""
SM2数字签名算法基础实现
符合GM/T 0003.2-2012标准

包含密钥生成、数字签名、签名验证功能
"""

import random
import hashlib
from typing import Tuple, Union, Optional

# 修复导入问题
try:
 from .ecc_math import SM2Curve, ECCPoint
 from .sm3_hash import sm3_hash
except ImportError:
 from ecc_math import SM2Curve, ECCPoint
 from sm3_hash import sm3_hash

class SM2Signature:
 """SM2数字签名算法实现"""

 def __init__(self):
 """初始化SM2签名对象"""
 self.curve = SM2Curve()
 self.user_id = "31323334353637383132333435363738" # 默认用户标识

 def _za_value(self, public_key: ECCPoint, user_id: str = None) -> bytes:
 """
 计算ZA值 (用户身份标识的杂凑值)

 Args:
 public_key: 用户公钥
 user_id: 用户标识 (16进制字符串)

 Returns:
 ZA值 (32字节)
 """
 if user_id is None:
 user_id = self.user_id

 # 用户标识长度 (位)
 id_len = len(user_id) * 4

 # 构造ZA输入数据
 za_data = []

 # ENTL: 用户标识长度 (2字节)
 za_data.append(id_len.to_bytes(2, byteorder='big'))

 # 用户标识
 za_data.append(bytes.fromhex(user_id))

 # 椭圆曲线参数 a, b (各32字节)
 za_data.append(self.curve.a.to_bytes(32, byteorder='big'))
 za_data.append(self.curve.b.to_bytes(32, byteorder='big'))

 # 基点G的坐标 (各32字节)
 za_data.append(self.curve.gx.to_bytes(32, byteorder='big'))
 za_data.append(self.curve.gy.to_bytes(32, byteorder='big'))

 # 用户公钥坐标 (各32字节)
 za_data.append(public_key.x.to_bytes(32, byteorder='big'))
 za_data.append(public_key.y.to_bytes(32, byteorder='big'))

 # 计算SM3杂凑
 za_input = b''.join(za_data)
 return sm3_hash(za_input)

 def _message_hash(self, message: Union[bytes, str], public_key: ECCPoint,
 user_id: str = None) -> int:
 """
 计算消息的杂凑值 e = H(ZA || M)

 Args:
 message: 待签名消息
 public_key: 签名者公钥
 user_id: 用户标识

 Returns:
 消息杂凑值 (整数)
 """
 if isinstance(message, str):
 message = message.encode('utf-8')

 # 计算ZA
 za = self._za_value(public_key, user_id)

 # 计算 e = H(ZA || M)
 hash_input = za + message
 hash_output = sm3_hash(hash_input)

 # 转换为整数
 e = int.from_bytes(hash_output, byteorder='big')
 return e

 def generate_keypair(self) -> Tuple[int, ECCPoint]:
 """
 生成SM2密钥对

 Returns:
 (private_key, public_key)
 """
 return self.curve.generate_keypair()

 def sign(self, message: Union[bytes, str], private_key: int,
 public_key: ECCPoint = None, user_id: str = None) -> Tuple[int, int]:
 """
 SM2数字签名

 Args:
 message: 待签名消息
 private_key: 签名私钥
 public_key: 签名公钥 (如果为None则从私钥计算)
 user_id: 用户标识

 Returns:
 (r, s) 签名值对
 """
 if public_key is None:
 public_key = self.curve.point_multiply(private_key, self.curve.G)

 # 计算消息杂凑值
 e = self._message_hash(message, public_key, user_id)

 while True:
 # 生成随机数 k ∈ [1, n-1]
 k = random.randint(1, self.curve.n - 1)

 # 计算椭圆曲线点 (x1, y1) = [k]G
 point = self.curve.point_multiply(k, self.curve.G)

 # 计算 r = (e + x1) mod n
 r = (e + point.x) % self.curve.n

 # 如果 r = 0 或 r + k = n，则重新生成k
 if r == 0 or (r + k) % self.curve.n == 0:
 continue

 # 计算 s = (1 + dA)^(-1) * (k - r * dA) mod n
 temp1 = (1 + private_key) % self.curve.n
 temp1_inv = self.curve.mod_inverse(temp1, self.curve.n)
 temp2 = (k - r * private_key) % self.curve.n
 s = (temp1_inv * temp2) % self.curve.n

 # 如果 s = 0，则重新生成k
 if s == 0:
 continue

 return (r, s)

 def verify(self, message: Union[bytes, str], signature: Tuple[int, int],
 public_key: ECCPoint, user_id: str = None) -> bool:
 """
 SM2签名验证

 Args:
 message: 原始消息
 signature: 签名值对 (r, s)
 public_key: 签名者公钥
 user_id: 用户标识

 Returns:
 True if 签名有效, False otherwise
 """
 r, s = signature

 # 检验 r, s ∈ [1, n-1]
 if not (1 <= r < self.curve.n and 1 <= s < self.curve.n):
 return False

 # 计算消息杂凑值
 e = self._message_hash(message, public_key, user_id)

 # 计算 t = (r + s) mod n
 t = (r + s) % self.curve.n

 # 如果 t = 0，则验证失败
 if t == 0:
 return False

 # 计算椭圆曲线点 (x1', y1') = [s]G + [t]PA
 point1 = self.curve.point_multiply(s, self.curve.G)
 point2 = self.curve.point_multiply(t, public_key)
 point_sum = self.curve.point_add(point1, point2)

 # 如果点为无穷远点，则验证失败
 if point_sum.is_infinity:
 return False

 # 计算 R = (e + x1') mod n
 R = (e + point_sum.x) % self.curve.n

 # 验证 R == r
 return R == r

 def sign_digest(self, digest: bytes, private_key: int) -> Tuple[int, int]:
 """
 对已有的摘要进行签名 (用于兼容其他哈希算法)

 Args:
 digest: 消息摘要 (32字节)
 private_key: 签名私钥

 Returns:
 (r, s) 签名值对
 """
 if len(digest) != 32:
 raise ValueError("摘要长度必须为32字节")

 # 将摘要转换为整数
 e = int.from_bytes(digest, byteorder='big')

 while True:
 # 生成随机数 k
 k = random.randint(1, self.curve.n - 1)

 # 计算椭圆曲线点
 point = self.curve.point_multiply(k, self.curve.G)

 # 计算 r
 r = (e + point.x) % self.curve.n

 if r == 0 or (r + k) % self.curve.n == 0:
 continue

 # 计算 s
 temp1 = (1 + private_key) % self.curve.n
 temp1_inv = self.curve.mod_inverse(temp1, self.curve.n)
 temp2 = (k - r * private_key) % self.curve.n
 s = (temp1_inv * temp2) % self.curve.n

 if s == 0:
 continue

 return (r, s)

 def verify_digest(self, digest: bytes, signature: Tuple[int, int],
 public_key: ECCPoint) -> bool:
 """
 验证摘要签名

 Args:
 digest: 消息摘要 (32字节)
 signature: 签名值对 (r, s)
 public_key: 签名者公钥

 Returns:
 True if 签名有效, False otherwise
 """
 if len(digest) != 32:
 raise ValueError("摘要长度必须为32字节")

 r, s = signature

 # 检验参数范围
 if not (1 <= r < self.curve.n and 1 <= s < self.curve.n):
 return False

 # 将摘要转换为整数
 e = int.from_bytes(digest, byteorder='big')

 # 计算验证值
 t = (r + s) % self.curve.n
 if t == 0:
 return False

 point1 = self.curve.point_multiply(s, self.curve.G)
 point2 = self.curve.point_multiply(t, public_key)
 point_sum = self.curve.point_add(point1, point2)

 if point_sum.is_infinity:
 return False

 R = (e + point_sum.x) % self.curve.n
 return R == r

def test_sm2_signature():
 """测试SM2数字签名算法"""
 print("测试SM2数字签名算法...")

 sm2 = SM2Signature()

 # 生成密钥对
 private_key, public_key = sm2.generate_keypair()
 print(f" 密钥对生成成功")
 print(f" 私钥: {hex(private_key)}")
 print(f" 公钥: {public_key}")

 # 测试消息
 message = "Hello, SM2 Digital Signature!"
 print(f" 测试消息: {message}")

 # 数字签名
 signature = sm2.sign(message, private_key, public_key)
 r, s = signature
 print(f" 签名生成成功")
 print(f" r: {hex(r)}")
 print(f" s: {hex(s)}")

 # 签名验证
 is_valid = sm2.verify(message, signature, public_key)
 assert is_valid, "签名验证失败"
 print(" 签名验证成功")

 # 测试错误签名
 wrong_signature = (r, (s + 1) % sm2.curve.n)
 is_invalid = sm2.verify(message, wrong_signature, public_key)
 assert not is_invalid, "错误签名应该验证失败"
 print(" 错误签名验证失败 (符合预期)")

 # 测试不同消息
 wrong_message = "Wrong message"
 is_invalid2 = sm2.verify(wrong_message, signature, public_key)
 assert not is_invalid2, "不同消息应该验证失败"
 print(" 不同消息验证失败 (符合预期)")

 # 测试摘要签名
 digest = sm3_hash(message.encode('utf-8'))
 digest_signature = sm2.sign_digest(digest, private_key)
 digest_valid = sm2.verify_digest(digest, digest_signature, public_key)
 assert digest_valid, "摘要签名验证失败"
 print(" 摘要签名验证成功")

 print("SM2数字签名算法测试完成！")

if __name__ == "__main__":
 test_sm2_signature()
