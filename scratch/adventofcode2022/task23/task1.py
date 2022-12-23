import sys
from enum import Enum
from collections import defaultdict


class Direction(Enum):
    NORTH = 0
    SOUTH = 1
    WEST = 2
    EAST = 3

def find_elf(elves, x, y):
    if (x, y) in elves:
            return True
    return False

class Elf:
    def __init__(self, x, y):
        self.x = x
        self.y = y
        self.state = Direction.NORTH

    def change_direction(self):
        self.state = Direction((self.state.value + 1) % 4)

    def propose(self, elves):
        # At first, find all elves around me
        all_around = list()
        for i in range(4):
            all_around += self.get_positions(Direction((self.state.value + i) % 4))
        for position in set(all_around):
            if find_elf(elves, position[0], position[1]):
                break
        else:
            return None, None

        for i in range(4):
            positions = self.get_positions(Direction((self.state.value + i) % 4))
            is_ok = True
            for pos in positions:
                if find_elf(elves, pos[0], pos[1]):
                    is_ok = False
            if is_ok and positions:
                return positions[0][0], positions[0][1]
        return None, None


    def get_positions(self, state):
        x = self.x
        y = self.y
        result = list()
        if state == Direction.NORTH:
            result.append((x, y-1))
            result.append((x-1, y-1))
            result.append((x+1, y-1))
        if state == Direction.SOUTH:
            result.append((x, y+1))
            result.append((x-1, y+1))
            result.append((x+1, y+1))
        if state == Direction.WEST:
            result.append((x-1, y))
            result.append((x-1, y+1))
            result.append((x-1, y-1))
        if state == Direction.EAST:
            result.append((x+1, y))
            result.append((x+1, y+1))
            result.append((x+1, y-1))

        # result2 = list()
        # for pair in result:
        #     if pair[0] < 0 or pair[1] < 0:
        #         continue
        #     if pair[0] >= sizex:
        #         continue
        #     if pair[1] >= sizey:
        #         continue
        #     result2.append(pair)
        return result





def read_file():
    elves = list()
    elves = dict()
    sizex, sizey = 0,0
    with open(sys.argv[1], "r") as f:
        lines = f.read().splitlines()
    sizex = len(lines[0])
    sizey = len(lines)
    for i in range(len(lines)):
        line = lines[i]
        for x in range(len(line)):
            if line[x] == '#':
                #elves.append(Elf(x, i))
                elves[(x,i)] = Elf(x, i)
    return elves


def dump_map(elves):
    # find max y and x
    y = 0
    x = 0
    for elf in elves:
        if elf[1] > y:
            y = elf[1]
        if elf[0] > x:
            x = elf[0]

    line = list("." * (x+1))
    world = list()
    for _ in range(y+1): world.append(line.copy())

    for elf in elves:
        world[elf[1]][elf[0]] = '#'
    for world_line in world:
        print(world_line)


def gen_map(sizex, sizey):
    world = list()
    line = [0 for i in range(sizex)]
    for i in range(sizey):
        world.append(line.copy())
    return world

def count_rectangle(elves):
    miny = 10000
    minx = 10000
    maxy = -1000
    maxx = -1000
    for elf in elves.values():
        if elf.y > maxy:
            maxy = elf.y
        if elf.x > maxx:
            maxx = elf.x

        if elf.x < minx:
            minx = elf.x
        if elf.y < miny:
            miny = elf.y

    places = (maxx - minx+ 1) * (maxy - miny + 1)
    return places - len(elves)

if __name__ == "__main__":
    elves = read_file()
    dump_map(elves)
    for iteration in range(1000):
        world = defaultdict(lambda: defaultdict(int)) #gen_map(sizex, sizey)
        moves = list()
        for _, elf in elves.items():
            elfx, elfy = elf.propose(elves)
            if elfx != None:
                moves.append([elfx, elfy, elf])
                world[elfy][elfx] += 1

        for move in moves:
            val = world[move[1]][move[0]]
            #print(f"{move[2].x}{move[2].y} proposes {move[0]}{move[1]}")
            if val == 0:
                raise Exception("uhoh")
            if val == 1:
                move[2].x = move[0]
                move[2].y = move[1]
        new_elves = dict()
        for elf in elves.values():
            elf.change_direction()
            new_elves[(elf.x, elf.y)] = elf
        elves = new_elves

        print(f"\nAfter iteration {iteration + 1} ")
        #dump_map(elves)
        print(len(moves))
    print("Result is")
    print(count_rectangle(elves))

