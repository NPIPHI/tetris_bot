import time

import numpy as np
import pyautogui

from Board import Board
from BoardImage import BoardImage
import Actions

expected_board: Board = None
last_move: (int,int,bool) = None

from subprocess import *

path = R"C:\Users\16182\PycharmProjects\tetris\cpp\cmake-build-release\findmove.exe"


def make_move(board: Board) -> None:
    board_str = board.encode()

    p = Popen(f"{path} {board_str}", stdout=PIPE, stdin=PIPE,
              stderr=PIPE)
    result = b''
    while result == b'':
        result = p.stdout.readline().strip()
    p.kill()
    move = [int(x) for x in result.split(b':')]
    swap = bool(move[2])
    topped = False
    for y in range(12):
        for x in range(10):
            if board.grid[x, y]:
                topped = True
    drop_delay = 0
    if topped:
        drop_delay = 0

    Actions.execute_transform((move[0], move[1], swap), board, delay=0.01, drop_delay=drop_delay)


def print_grid(grid: np.ndarray):
    for y in range(grid.shape[1]):
        for x in range(grid.shape[0]):
            if grid[x, y]:
                print('x ', end='')
            else:
                print('. ', end='')
        print()


board_image = BoardImage()
Actions.focus()
Actions.swap_hold()
Actions.drop()
pyautogui.sleep(0.1)
while True:
    try:
        board_image.refresh()
        board = Board.from_image(board_image)

        make_move(board)
        pyautogui.sleep(0.03)
    except:
        print("bad read")
        break


