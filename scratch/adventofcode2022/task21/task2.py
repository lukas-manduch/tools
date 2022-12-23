import sys
import re

real_values = {}
operations = list() # [name, name, op, name]


def parse_line(line):
    line = line[0: -1]
    rematch = re.fullmatch(r"(\w+): (\d+)", line)
    if rematch:
        if rematch [1] == 'humn':
            return
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
        return True
    return False

def go_through2():
    for i in range(len(operations)):
        op = operations[i]
        if not op[0] in real_values:
            continue
        result = real_values[op[0]]
        second = False
        if not op[1] in real_values and not op[3] in real_values:
            raise Exception(f"Cannot solve{dump_expr(op)}")
        if op[1] in real_values:
            operand = real_values[op[1]]
        if op[3] in real_values:
            operand = real_values[op[3]]
            second = True

        if op[2] == '+':
            if second:
                real_values[op[1]] = result - operand
            else:
                real_values[op[3]] = result - operand

        if op[2] == '*':
            #print(result/ operand)
            if second:
                real_values[op[1]] = result / operand
            else:
                real_values[op[3]] = result / operand
        if op[2] == '-':
            if second:
                real_values[op[1]] =  operand + result
            else:
                real_values[op[3]] = operand - result
        if op[2] == '/':
            if second:
                real_values[op[1]] = result * operand
            else:
                real_values[op[3]] = operand / result
        print(f"{real_values[op[0]]}  = {real_values[op[1]]} {op[2]} {real_values[op[3]]} {second}")

        del operations[i]
        return True
    return False



def dump_expr(exp):
    print(exp)
    if exp[1] in real_values:
        print(f"1st is {real_values[exp[1]]}")
    if exp[3] in real_values:
        print(f"2nd is {real_values[exp[3]]}")


with open(sys.argv[1], "r") as f:
    for line in f.readlines():
        parse_line(line)
    while go_through():
        pass

    real_values['fglq'] = 42130890593816
    while go_through2():
        pass
    print(f"Remaining exps: {len(operations)}")
    dump_expr(operations[0])
    print(real_values['humn'])
        
    #print(real_values['root'])
