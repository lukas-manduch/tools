import sys
import math

snafud = {
        '2' : 2,
        '1' : 1,
        '0' : 0,
        '-' : -1,
        '=' : -2,
        }

dsnafu = {
        0: '0',
        1: '1',
        2: '2',
        3: '=',
        4: '-',
        }

def from_snafu(number):
    result = 0
    for i, val in enumerate(reversed(number)):
        multip = math.pow(5,i)
        result += snafud[val] * multip
    return result

def to_snafu(value):
    result = ''
    index = -1
    while value > 0:
        index += 1
        currentm = pow(5, index)
        nextm = pow(5, index+1)

        to_solve = (value % nextm) / currentm

        if to_solve < 0 or to_solve > 4:
            raise Exception("Bad")

        char = dsnafu[to_solve]
        value -= snafud[char] * currentm

        result += char


    if value != 0:
        raise Exception("Uh oh")
    return result[::-1]

def test():
    for i in range(100):
        print(f"val: {i}   =  {to_snafu(i)}")

def process_file():
    with open(sys.argv[1], "r") as f:
        lines = f.read().splitlines()
    value = 0
    for line in lines:
        value += from_snafu(line)
    return value



if __name__ == "__main__":
    value = process_file()
    print(value)
    print(to_snafu(value))



