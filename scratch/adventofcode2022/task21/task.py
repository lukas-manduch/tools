import sys
import re

real_values = {}
operations = list() # [name, name, op, name]


def parse_line(line):
    line = line[0: -1]
    rematch = re.fullmatch(r"(\w+): (\d+)", line)
    if rematch:
        real_values[rematch[1]] = int(rematch[2])
    else:
        rematch = re.fullmatch(r"(\w+): (\w+)\s*([+-/*])\s*(\w+)", line)
        if not rematch:
            raise Exception("Bad regex")
        operations.append([rematch[1], rematch[2], rematch[3], rematch[4]])

def go_through():
    # print(len(operations))
    for i in range(len(operations)):
        op = operations[i]
        if not op[1] in real_values:
            continue
        if not op[3] in real_values:
            continue
        print("YES")
        if op[2] == '+':
            real_values[op[0]] = real_values[op[1]] + real_values[op[3]]
        if op[2] == '-':
            real_values[op[0]] = real_values[op[1]] - real_values[op[3]]
        if op[2] == '*':
            real_values[op[0]] = real_values[op[1]] * real_values[op[3]]
        if op[2] == '/':
            real_values[op[0]] = real_values[op[1]] / real_values[op[3]]
        del operations[i]
        break


with open(sys.argv[1], "r") as f:
    for line in f.readlines():
        parse_line(line)
    while operations:
        go_through()
    print(real_values['root'])
