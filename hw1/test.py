import os
import random
import subprocess
import time
import numpy as np

result_file = open('res.txt', 'w')
expect = []
ops = []

name = input("Please input your program's name:")
flag = int(input("Please input your test range(1 for no negative, 0 for all):"))
proc = subprocess.Popen('./'+name, shell = True, stdin = subprocess.PIPE, stdout=result_file)
for i in range(0, 100):
    op1 = random.randint(-10000000 if flag == 0 else 0, 100000000)
    op2 = random.randint(-10000000 if flag == 0 else 0, 1000000000)
    ops.append([op1, op2])
    expect_add = op1 + op2
    expect_mul = op1 * op2
    op1 = str(op1).encode('utf-8')
    op2 = str(op2).encode('utf-8')
    expect.append(str(expect_add))
    expect.append(str(expect_mul))
    proc.stdin.write(op1+b'+'+op2+b'\n')
    proc.stdin.flush()
    time.sleep(0.01)
    proc.stdin.write(op1+b'*'+op2+b'\n')
    proc.stdin.flush()
    time.sleep(0.01)
proc.stdin.write(b'q\n')
proc.stdin.flush()
proc.stdin.close()

result_file.close()
time.sleep(2)
result_file = open("res.txt", 'r')
result_lines = result_file.readlines()
failure = 0
for i in range(0, len(result_lines)):
    if expect[i] != result_lines[i].strip():
        operator = '*'
        if i % 2 == 0:
            operator = '+'
        print("wrong result: "+str(ops[i//2][0])+operator+str(ops[i//2][1]))
        print("expect: "+expect[i])
        print("actual: "+str(result_lines[i]))
        failure += 1
if failure == 0:
    print('\033[92m'+"All tests passed!")

    