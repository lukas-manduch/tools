import sys

class Point:

    def __init__(self):
        self.down = False
        self.up = False
        self.right = False
        self.left = False
        self.wall = False

    def empty(self):
        return not (self.down or self.up or self.left or self.right or self.wall)

    def multi(self):
        val = int(self.down) + int(self.up) + int(self.left) + int(self.right)
        return val

def load_map():
    with open(sys.argv[1], "r") as f:
        lines = f.read().splitlines()
    world = gen_map(len(lines[0]), len(lines))
    for y in range(len(lines)):
        line = lines[y]
        for x in range(len(line)):
            char = line[x]
            point = world[y][x]
            if char == '<':
                point.left = True
            elif char == '>':
                point.right = True
            elif char == 'v':
                point.down = True
            elif char == '^':
                point.up = True
    return world

def print_map(world):
    for line in world:
        for point in line:
            if point.empty():
                print(".", end='')
            elif point.wall:
                print('#', end='')
            elif point.multi() > 1:
                print(point.multi(), end='')
            elif point.down:
                print("v", end='')
            elif point.up:
                print("^", end='')
            elif point.left:
                print("<", end='')
            elif point.right:
                print(">", end='')
        print()

def gen_map(sizex, sizey):
    world = list()
    for y in range(sizey):
        line = list()
        for x in range(sizex):
            line.append(Point())
            if y == 0 or y == (sizey-1) or x == 0 or x == (sizex - 1):
                line[-1].wall = True
        world.append(line)
    world[0][1].wall = False
    world[-1][-2].wall = False
    return world

def maxx(world):
    return len(world[0]) - 2

def maxy(world):
    return len(world) - 2

def minx(world):
    return 1

def miny(world):
    return 1

def next_state(world):
    new_state = gen_map(len(world[0]), len(world))
    for y in range(len(world)):
        for x in range(len(world[y])):
            point = world[y][x]
            if point.left == True:
                newx = maxx(world) if x == minx(world) else x-1
                new_state[y][newx].left = True
            if point.right:
                newx = minx(world) if x == maxx(world) else x+1
                new_state[y][newx].right = True
            if point.down:
                newy = miny(world) if y == maxy(world) else y + 1
                new_state[newy][x].down = True
            if point.up:
                newy = maxy(world) if y == miny(world) else y - 1
                new_state[newy][x].up = True
    return new_state

def is_valid_position(x, y, world):
    if x < 0 or y < 0:
        return False
    if x >= len(world[0]) or y >= len(world):
        return False
    return world[y][x].empty()

def solve(states):
    trip = 1
    positions = [(1, 0)]
    final_position = (len(states[0][0]) - 2, len(states[0]) - 1)
    final_bak = final_position

    for index, world in enumerate(states[1:]):
        new_positions = list()
        for old_position in positions:

            tmp  = old_position[0], old_position[1]
            if is_valid_position(tmp[0], tmp[1], world):
                new_positions.append(tmp)

            tmp  = old_position[0] - 1, old_position[1]
            if is_valid_position(tmp[0], tmp[1], world):
                new_positions.append(tmp)

            tmp  = old_position[0] + 1, old_position[1]
            if is_valid_position(tmp[0], tmp[1], world):
                new_positions.append(tmp)

            tmp  = old_position[0], old_position[1] + 1
            if is_valid_position(tmp[0], tmp[1], world):
                new_positions.append(tmp)

            tmp  = old_position[0], old_position[1] - 1
            if is_valid_position(tmp[0], tmp[1], world):
                new_positions.append(tmp)

        # print(f"Iteration {index + 1} positions {new_positions}")
        # print_map(world)
        # print()

        if index % 50 == 0:
            print(f"On index {index}")

        for position in new_positions:
            if position == final_position:
                print(f"Found solution on index {index} {position}")
                if trip == 1:
                    trip = 2
                    new_positions = [ final_position ]
                    final_position = (1, 0)
                    break
                elif trip == 2:
                    trip = 3
                    new_positions = [ final_position ]
                    final_position = final_bak
                    break
                else:
                    exit(0)
        positions = list(set(new_positions))


if __name__ == "__main__":
    states = list()
    states.append(load_map())
    for i in range(900):
        states.append(next_state(states[-1]))

    solve(states)

