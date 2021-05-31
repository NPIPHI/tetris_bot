import pyautogui
from pyautogui import *

from Board import Board
from Piece import Piece

pyautogui.PAUSE = 0

left_pad_table = {
    'line': (3, 5, 3, 4),
    'T': (3, 4, 3, 3),
    'bZ': (3, 4, 3, 3),
    'square': (4, 4, 4, 4),
    'bL': (3, 4, 3, 3),
    'L': (3, 4, 3, 3),
    'Z': (3, 4, 3, 3)
}


def focus():
    moveTo(1200, 600)
    click()


def swap_hold():
    typewrite('c')


def translate_arr(dx: int):
    if dx < 0:
        return [LEFT] * -dx
    else:
        return [RIGHT] * dx


def execute_transform(transform: (int, int, bool), board: Board):
    command_buffer = []
    piece = board.hold_piece if transform[2] else board.current_piece
    if transform[2]:
        typewrite('c', interval=0.01)
        # command_buffer.append('c')

    dx = transform[0] - left_pad_table[piece.type][transform[1]]
    typewrite(rotate_arr(transform[1]), interval=0.01)
    typewrite(translate_arr(dx), interval=0.01)
    typewrite(' ', interval=0)
    # command_buffer += rotate_arr(transform[1])
    # command_buffer += translate_arr(dx)
    # command_buffer += [' ']
    # typewrite(command_buffer, interval=0.01)


def rotate_arr(dtheta: int):
    dtheta = dtheta % 4
    if dtheta == 0:
        return []
    elif dtheta == 1:
        return ['x']
    elif dtheta == 2:
        return ['x', 'x']
    elif dtheta == 3:
        return ['z']


def drop():
    write(' ')
