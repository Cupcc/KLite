#!python
import io
import os
import re
import sys

def check_line_start(line):
	for ch in line:
		if(ch == " "):
			return False
		if(ch != "\t"):
			return True
	
def check_line_end(line):
	if line.endswith(" \r\n"):
		return False
	if line.endswith("\t\r\n"):
		return False
	return True
	
def check_line_crlf(line):
	cr = 0
	lf = 0
	for ch in line:
		if(ch == "\r"):
			cr = cr + 1
		if(ch == "\n"):
			lf = lf + 1
	if cr == 1 and lf == 1:
		return True
	return False
	
def check_space_line(line):
	for ch in line:
		if ch == " ":
			continue
		if ch == "\t":
			continue
		if ch == "\r":
			continue
		if ch == "\n":
			return False
		return True
	return False
	
def check_style(path):
	#print(path)
	file = io.open(path, "rb")
	line_num = 0
	space_line = 0
	for line in file:
		#print(line)
		line = line.decode('latin1')
		#print(line)
		line_num = line_num + 1
		if not check_space_line(line):
			space_line = space_line + 1
			if(space_line > 1):
				print(path, "第%d行:" % (line_num), "连续空行")
			continue
		else:
			space_line = 0
		if not check_line_crlf(line):
			print(path, "第%d行:" % (line_num), "换行不规范")
			continue
		if not check_line_start(line):
			print(path, "第%d行:" % (line_num), "行首有空格")
			continue
		if not check_line_end(line):
			print(path, "第%d行:" % (line_num), "行尾有空格")
			continue
	file.close()
	
def scan_dir_files(dir):
	list = os.listdir(dir)
	for item in list:
		path = dir + "\\" + item
		if os.path.isfile(path):
			check_style(path)
		if os.path.isdir(path):
			scan_dir_files(path)
	return None

def main():
	if len(sys.argv) != 2:
		print("python check_style.py <path>")
		return False
	scan_dir_files(sys.argv[1])
	print("检查完毕")
	
main()
