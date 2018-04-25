#!/usr/bin/env python3

# Find the largest rectangle in O(n²)

import sys, random

n, m = 8, 8
if len(sys.argv) > 1:
    n = int(sys.argv[1])
    m = n
if len(sys.argv) > 2:
    m = int(sys.argv[2])
if len(sys.argv) > 3:
    sys.exit(1)

class Cell:
    def __init__(self, active):
        self.active = active
        self.up = int(active)

rand = random.Random()
board = [[Cell(rand.randint(0, 1) == 1) for x in range(m)] + [Cell(False)] for y in range(n)]

for y in range(1, n):
    for x in range(0, m):
        if board[y][x].active:
            board[y][x].up = board[y - 1][x].up + 1

best = 0
for y in range(n):
    stack = []
    for x in range(m + 1):
        cell = board[y][x]
        if len(stack) and stack[-1][1] < cell.up:
            stack.append((x, cell.up))
        x0 = x
        while len(stack) and stack[-1][1] > cell.up:
            (x0, h) = stack.pop()
            w = x - x0
            best = max(best, w * h)
        if not len(stack) or stack[-1][1] < cell.up:
            stack.append((x0, cell.up))

for y in range(n):
    row = board[y]
    for x in range(m):
        cell = row[x]
        if cell.active:
            print('\033[7m  \033[m', end = '', file = sys.stderr)
        else:
            print('  ', end = '', file = sys.stderr)
    print(file = sys.stderr)

print(best)
