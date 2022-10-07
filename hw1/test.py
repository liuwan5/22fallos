import os
import subprocess

result_file = open('res.txt', 'w')
test_file = open('测试（不含负数）.txt', 'r')
test_lines = test_file.readlines()
expect = []
    
proc = subprocess.Popen('./hw1.out', shell = True, stdin = subprocess.PIPE, stdout=result_file)
for i in range(0, len(test_lines), 4):
    op1, op2 = test_lines[i].split()
    op1 = op1.encode('utf-8')
    op2 = op2.encode('utf-8')
    expect.append(test_lines[i+1].strip())
    expect.append(test_lines[i+2].strip())
    proc.stdin.write(op1+b'+'+op2+b'\n')
    proc.stdin.write(op1+b'*'+op2+b'\n')
proc.stdin.write(b'q\n')
proc.stdin.flush()
proc.stdin.close()

result_file.close()
result_file = open("res.txt", 'r')
result_lines = result_file.readlines()
failure = 0
for i in range(0, len(result_lines)):
    if expect[i] != result_lines[i].strip():
        print("wrong in line"+str(i))
        failure += 1
if failure == 0:
    print('\033[92m'+"All tests passed!")

    