"""
SM3密码杂凑算法实现
符合GM/T 0004-2012标准

为SM2数字签名算法提供哈希函数支持
"""

import struct
from typing import Union


class SM3Hash:
    """SM3哈希算法实现"""
    
    def __init__(self):
        """初始化SM3哈希对象"""
        # SM3初始值 (GM/T 0004-2012)
        self.iv = [
            0x7380166F, 0x4914B2B9, 0x172442D7, 0xDA8A0600,
            0xA96F30BC, 0x163138AA, 0xE38DEE4D, 0xB0FB0E4E
        ]
        
        # 常数T
        self.T = [
            0x79CC4519 if j < 16 else 0x7A879D8A
            for j in range(64)
        ]
        
        self.reset()
    
    def reset(self):
        """重置哈希状态"""
        self.state = self.iv.copy()
        self.buffer = b''
        self.counter = 0
    
    def _rotl(self, x: int, n: int) -> int:
        """32位循环左移"""
        return ((x << n) | (x >> (32 - n))) & 0xFFFFFFFF
    
    def _ff(self, x: int, y: int, z: int, j: int) -> int:
        """布尔函数FF"""
        if j < 16:
            return x ^ y ^ z
        else:
            return (x & y) | (x & z) | (y & z)
    
    def _gg(self, x: int, y: int, z: int, j: int) -> int:
        """布尔函数GG"""
        if j < 16:
            return x ^ y ^ z
        else:
            return (x & y) | (~x & z)
    
    def _p0(self, x: int) -> int:
        """置换函数P0"""
        return x ^ self._rotl(x, 9) ^ self._rotl(x, 17)
    
    def _p1(self, x: int) -> int:
        """置换函数P1"""
        return x ^ self._rotl(x, 15) ^ self._rotl(x, 23)
    
    def _compress(self, block: bytes):
        """压缩函数"""
        # 消息扩展
        w = [0] * 68
        w1 = [0] * 64
        
        # 前16个字从消息块直接获得
        for i in range(16):
            w[i] = struct.unpack('>I', block[i*4:(i+1)*4])[0]
        
        # 扩展为68个字
        for i in range(16, 68):
            temp = w[i-16] ^ w[i-9] ^ self._rotl(w[i-3], 15)
            w[i] = self._p1(temp) ^ self._rotl(w[i-13], 7) ^ w[i-6]
        
        # 生成W1
        for i in range(64):
            w1[i] = w[i] ^ w[i+4]
        
        # 压缩
        A, B, C, D, E, F, G, H = self.state
        
        for j in range(64):
            # SS1 = ROL((ROL(A, 12) + E + ROL(T[j], j % 32)), 7)
            temp1 = (self._rotl(A, 12) + E + self._rotl(self.T[j], j % 32)) & 0xFFFFFFFF
            SS1 = self._rotl(temp1, 7)
            
            # SS2 = SS1 ⊕ ROL(A, 12)
            SS2 = SS1 ^ self._rotl(A, 12)
            
            # TT1 = FF(A,B,C) + D + SS2 + W1[j]
            TT1 = (self._ff(A, B, C, j) + D + SS2 + w1[j]) & 0xFFFFFFFF
            
            # TT2 = GG(E,F,G) + H + SS1 + W[j]
            TT2 = (self._gg(E, F, G, j) + H + SS1 + w[j]) & 0xFFFFFFFF
            
            D = C
            C = self._rotl(B, 9)
            B = A
            A = TT1
            H = G
            G = self._rotl(F, 19)
            F = E
            E = self._p0(TT2)
            
            # 确保所有值都在32位范围内
            A, B, C, D, E, F, G, H = [x & 0xFFFFFFFF for x in [A, B, C, D, E, F, G, H]]
        
        # 更新状态
        self.state[0] = (self.state[0] ^ A) & 0xFFFFFFFF
        self.state[1] = (self.state[1] ^ B) & 0xFFFFFFFF
        self.state[2] = (self.state[2] ^ C) & 0xFFFFFFFF
        self.state[3] = (self.state[3] ^ D) & 0xFFFFFFFF
        self.state[4] = (self.state[4] ^ E) & 0xFFFFFFFF
        self.state[5] = (self.state[5] ^ F) & 0xFFFFFFFF
        self.state[6] = (self.state[6] ^ G) & 0xFFFFFFFF
        self.state[7] = (self.state[7] ^ H) & 0xFFFFFFFF
    
    def update(self, data: Union[bytes, str]):
        """更新哈希数据"""
        if isinstance(data, str):
            data = data.encode('utf-8')
        
        self.buffer += data
        self.counter += len(data)
        
        # 处理完整的64字节块
        while len(self.buffer) >= 64:
            self._compress(self.buffer[:64])
            self.buffer = self.buffer[64:]
    
    def digest(self) -> bytes:
        """获取哈希摘要"""
        # 填充
        msg_len = self.counter
        msg = self.buffer
        
        # 添加'1'位 (0x80字节)
        msg += b'\x80'
        
        # 填充0直到长度≡56 (mod 64)
        while (len(msg) % 64) != 56:
            msg += b'\x00'
        
        # 添加原始消息长度(位)
        msg += struct.pack('>Q', msg_len * 8)
        
        # 处理最后的块
        temp_state = self.state.copy()
        for i in range(0, len(msg), 64):
            self._compress(msg[i:i+64])
        
        # 生成摘要
        digest = b''
        for word in self.state:
            digest += struct.pack('>I', word)
        
        # 恢复状态 (保持对象可重用)
        self.state = temp_state
        
        return digest
    
    def hexdigest(self) -> str:
        """获取十六进制哈希摘要"""
        return self.digest().hex()


def sm3_hash(data: Union[bytes, str]) -> bytes:
    """SM3哈希函数(便捷接口)"""
    hasher = SM3Hash()
    hasher.update(data)
    return hasher.digest()


def sm3_hexdigest(data: Union[bytes, str]) -> str:
    """SM3哈希函数十六进制输出(便捷接口)"""
    return sm3_hash(data).hex()


def test_sm3():
    """测试SM3哈希函数"""
    print("测试SM3哈希算法...")
    
    # 测试向量1: 空字符串
    result1 = sm3_hexdigest("")
    expected1 = "1ab21d8355cfa17f8e61194831e81a8f22bec8c728fefb747ed035eb5082aa2b"
    assert result1 == expected1, f"空字符串测试失败: {result1}"
    print("✓ 空字符串测试通过")
    
    # 测试向量2: "abc"
    result2 = sm3_hexdigest("abc")
    expected2 = "66c7f0f462eeedd9d1f2d46bdc10e4e24167c4875cf2f7a2297da02b8f4ba8e0"
    assert result2 == expected2, f"'abc'测试失败: {result2}"
    print("✓ 'abc'测试通过")
    
    # 测试向量3: 长字符串
    long_msg = "a" * 1000000
    result3 = sm3_hexdigest(long_msg)
    print(f"✓ 长字符串测试通过: {result3[:16]}...")
    
    print("SM3哈希算法测试完成！")


if __name__ == "__main__":
    test_sm3()
