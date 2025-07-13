import os
import re
import sys

def remove_emojis_from_file(filepath):
 """删除文件中的所有emoji"""
 try:
 with open(filepath, 'r', encoding='utf-8') as f:
 content = f.read()

 # 定义emoji模式
 emoji_pattern = re.compile(
 r'[]'
 )

 # 删除emoji，但保留前面的空格
 original_content = content
 content = emoji_pattern.sub('', content)

 # 清理多余的空行和空格
 content = re.sub(r'\n\s*\n\s*\n', '\n\n', content) # 最多保留一个空行
 content = re.sub(r' +', ' ', content) # 多个空格合并为一个
 content = re.sub(r' +\n', '\n', content) # 删除行尾空格

 # 只有在内容发生变化时才写入文件
 if content != original_content:
 with open(filepath, 'w', encoding='utf-8') as f:
 f.write(content)
 print(f"处理完成: {filepath}")
 return True
 else:
 return False

 except Exception as e:
 print(f"处理文件 {filepath} 时出错: {e}")
 return False

def process_directory(directory):
 """递归处理目录中的所有文件"""
 processed_count = 0
 total_count = 0

 # 支持的文件扩展名
 supported_extensions = {'.md', '.py', '.js', '.txt', '.json', '.ts', '.jsx', '.tsx'}

 for root, dirs, files in os.walk(directory):
 # 跳过一些不需要处理的目录
 dirs[:] = [d for d in dirs if not d.startswith('.') and d not in ['node_modules', '__pycache__', 'build']]

 for file in files:
 filepath = os.path.join(root, file)
 _, ext = os.path.splitext(file)

 if ext.lower() in supported_extensions:
 total_count += 1
 if remove_emojis_from_file(filepath):
 processed_count += 1

 print(f"\n处理完成！")
 print(f"总共检查文件: {total_count}")
 print(f"修改文件数量: {processed_count}")

if __name__ == "__main__":
 base_dir = r"d:\OneDrive\桌面\cybersec_project"
 print(f"开始处理目录: {base_dir}")
 process_directory(base_dir)
