from enum import Enum
import sys

world = list()
instructions_raw = ''
instructions = list()

class Direction(Enum):
    LEFT = 0
    UP = 1
    RIGHT = 2
    DOWN = 3

class Instruction:
    def __init__(self, distance, direction):
        self.distance = distance
        self.direction = direction

    def __str__(self):
        if self.distance:
            return f"DISTANCE {self.distance}"
        return f"Direction {self.direction}"

with open(sys.argv[1], "r") as f:
    lines = f.read().splitlines()
    for line in lines:
        if len(line) > 0:
            world.append(line)
        else:
            break
    instructions_raw = lines[-1]

    longest_line = max(map(len, world))

    for i in range(len(world)):
        line = world[i]
        if len(line) < longest_line:
            missing_spaces = (longest_line - len(line)) * ' '
            line = line + missing_spaces
            world[i] = line

tmp = ''
for char in instructions_raw:
    if char.isdigit():
        tmp += char
    else:
        if tmp:
            instructions.append(Instruction(int(tmp), 3))
            tmp = ''

        if char == 'R':
            instructions.append(Instruction(0, Direction.RIGHT))
        elif char == 'L':
            instructions.append(Instruction(0, Direction.LEFT))
        # elif char == 'U':
        #     instructions.append(Instruction(0, Direction.UP))
        # elif char == 'D':
        #     instructions.append(Instruction(0, Direction.DOWN))
        else:
            raise Exception("Not valid instruction")
if tmp:
        instructions.append(Instruction(int(tmp), None))
        tmp = ''


# for instr in instructions:
#     print(instr)

def do_op(direction, x, y):
    if direction == Direction.UP:
        return x, y-1
    elif direction == Direction.DOWN:
        return x, y+1
    elif direction == Direction.RIGHT:
        return x+1, y
    elif direction == Direction.LEFT:
        return x-1, y
    else:
        raise Exception(f"Invalid op {direction}")

def new_position(direction, x, y):
    nx, ny = x, y

    while True:
        nx,ny = do_op(direction, nx, ny)

        nx = nx % len(world[0])
        ny = ny % len(world)

        try:
            if world[ny][nx] == ' ':
                continue
            elif world[ny][nx] == '#':
                return x,y
            elif world[ny][nx] == '.':
                return nx, ny
            else:
                raise Exception("Map wrong")
        except:
            breakpoint()
            print(len(world))
            print(ny)
            print(nx)
            raise

def new_direction(direction, op):
    value = direction.value
    if op == Direction.RIGHT:
        value += 1
    else:
        value -= 1
    return Direction(value%4)

def solve():
    positionx = world[0].find('.')
    positiony = 0
    direction = Direction.RIGHT

    for instr in instructions:
        if instr.distance:
            for i in range(instr.distance):
                positionx, positiony = new_position(direction, positionx, positiony)
        else:
            direction = new_direction(direction, instr.direction)
        #print(f"{positionx} {positiony}")
    print(f"column {positionx} {4 * (positionx + 1)}")
    print(f"row {positiony} {1000 * (positiony + 1)}")
    print(direction)

print(len(world[0]))
solve()
print(Direction(1).value)
