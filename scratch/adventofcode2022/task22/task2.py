from enum import Enum
import sys

world = list()
instructions_raw = ''
instructions = list()

WIDTH=50

class Direction(Enum):
    LEFT = 0
    UP = 1
    RIGHT = 2
    DOWN = 3

def get_id(x, y):
    idx = x // WIDTH
    idy = y // WIDTH
    return idx, idy


class Destination:
    def __init__(self, cubeid, selectorx, selectory, direction):
        self.cubeid = cubeid
        self.selectorx = selectorx
        self.selectory = selectory
        self.direction = direction

    def process(self, x, y):
        newx = 0
        newy = 0
        ndirection = None
        if self.selectorx == 'l':
            newx = self.cubeid[0]*WIDTH
        elif self.selectorx == 'r':
            newx = (self.cubeid[0] + 1) * WIDTH - 1
        elif self.selectorx == 'y':
            newx = y % WIDTH
            newx += self.cubeid[0]*WIDTH
        elif self.selectorx == 'x':
            newx = x % WIDTH
            newx += self.cubeid[0] * WIDTH

        else:
            raise Exception("x not impl")

        if self.selectory == 'x':
            newy = x % WIDTH
            newy = newy + self.cubeid[1]*WIDTH
        elif self.selectory == '-y':
            newy = WIDTH - 1 - (y % WIDTH)
            newy += self.cubeid[1] * WIDTH

        elif self.selectory == 'd':
            newy = (self.cubeid[1] + 1) * WIDTH - 1
        elif self.selectory == 'u':
            newy = self.cubeid[1] * WIDTH
        else:
            raise Exception("y not impl")
        print(f"New dest {x} , {y} To {newx} , {newy}{ndirection}")
        return self.direction, newx, newy


teleports = {
        (1,0) : {
            (1, -1): Destination((0, 3), 'l', 'x', Direction.RIGHT),
            (0,0): Destination((0,2), 'l', '-y', Direction.RIGHT),
            },

        (0,2) : {
            (0,1): Destination((1,1), 'l', 'x', Direction.RIGHT),
            (-1, 2): Destination((1,0), 'l', '-y', Direction.RIGHT),
            },
        (2,0) : {
            (2,1): Destination((1,1), 'r', 'x', Direction.LEFT),
            (2, -1): Destination((0,3), 'x', 'd', Direction.UP ),
            (3, 0): Destination((1, 2), 'r', '-y', Direction.LEFT),
            },
        (1,1) : {
            (2,1): Destination((2,0), 'y', 'd', Direction.UP),
            (0,1): Destination((0, 2), 'y', 'u', Direction.DOWN),
            },
        (0,3) : {
            (-1, 3): Destination((1, 0), 'y', 'u', Direction.DOWN),
            (0, 4):  Destination((2, 0), 'x', 'u', Direction.DOWN),
            (1, 3):  Destination((1, 2), 'y', 'd', Direction.UP),
            },
        (1, 2): {
            (2, 2): Destination((2, 0), 'r', '-y', Direction.LEFT),
            (1, 3): Destination((0, 3), 'r', 'x', Direction.LEFT) },
}


def teleport(oldx, oldy, newx, newy):
    idold = get_id(oldx, oldy)
    idnew = get_id(newx, newy)
    if not idold in teleports or not idnew in teleports[idold]:
        print(f"Missing teleport for {idold} [{oldx}, {oldy}]] {idnew} [{newx} {newy}] ")
        print(teleports)
        exit(1)

    teleporter = teleports[idold][idnew]
    return teleporter.process(newx, newy)

    #return Direction.LEFT, newx, newy

class Instruction:
    def __init__(self, distance, direction):
        self.distance = distance
        self.direction = direction

    def __str__(self):
        if self.distance:
            return f"DISTANCE {self.distance}"
        return f"Direction {self.direction}"

def load_world():
    with open(sys.argv[1], "r") as f:
        lines = f.read().splitlines()
        for line in lines:
            if len(line) > 0:
                world.append(line)
            else:
                break
        instructions_raw = lines[-1]
        longest_line = max(map(len, world))

    # Parse intructions
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
            else:
                raise Exception("Not valid instruction")
    if tmp:
            instructions.append(Instruction(int(tmp), None))
            tmp = ''


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
    nx,ny = do_op(direction, nx, ny)
    ndirection = direction
    if ny >= len(world) or ny < 0 or nx >= len(world[ny]) or nx < 0 or world[ny][nx] == ' ':
        ndirection, nx, ny = teleport(x, y, nx, ny)

    if world[ny][nx] == '#':
        return direction, x, y
    elif world[ny][nx] == '.':
        return ndirection, nx, ny
    else:
        raise Exception("Map wrong")

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
                direction, positionx, positiony = new_position(direction, positionx, positiony)
        else:
            direction = new_direction(direction, instr.direction)
    print(f"column {positionx} {4 * (positionx + 1)}")
    print(f"row {positiony} {1000 * (positiony + 1)}")
    print(direction)

if __name__ == "__main__":
    load_world()
    solve()



