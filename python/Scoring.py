import math

import numpy as np
from Board import Board
from Piece import Piece

line_clear_values = [
    0,
    0,
    1000,
    5000,
    20000,
]

board_height_values = [
    -14000,
    -13000,
    -12000,
    -11000,
    -10000,
    -90000,
    -80000,
    -60000,
    -40000,
    -20000,
    -10000,
    -5000,
    -1000,
    -500,
    -300,
    0,
    300,
    400,
    500,
    500,
    500,
    400,
    100,
    -500,
    -1000
]


def count_holes(grid: np.ndarray) -> int:
    total_holes = 0
    for x in range(grid.shape[0]):
        has_roof = False
        for y in range(grid.shape[1]):
            if grid[x, y]:
                has_roof = True
            else:
                if has_roof:
                    total_holes += 1
                has_roof = False
    return total_holes


def get_top_occupied(grid: np.ndarray) -> int:
    for y in range(grid.shape[1]):
        for x in range(grid.shape[0]):
            if grid[x, y]:
                return y
    return 24


def center_of_mass(grid: np.ndarray) -> float:
    total_mass = 0
    total_blocks = 0
    for x in range(grid.shape[0]):
        for y in range(grid.shape[1]):
            if(grid[x,y]):
                total_mass += y
                total_blocks += 1

    if(total_blocks == 0):
        return 24
    return total_mass/total_blocks


def computer_score(board: np.ndarray) -> int:
    total_holes = count_holes(board)
    top_occupied = get_top_occupied(board)

    total_score = total_holes * -10000 \
                  + board_height_values[top_occupied] \
                  + 100 * center_of_mass(board)
    return total_score


def score_recur(board: Board, cleard_lines: [int], depth: int) -> int:
    if depth <= 1:
        return computer_score(board.grid) + sum(map(lambda x: line_clear_values[x], cleard_lines))
    else:
        ctransforms = legal_transforms(board.current_piece)
        stransforms = legal_transforms(board.hold_piece)
        all_transforms = [(a[0],a[1],False) for a in ctransforms] + [(a[0],a[1],True) for a in stransforms]
        best_score = -math.inf
        for trans in all_transforms:
            b, cleared = apply_transform(board, trans)
            score = score_recur(b, cleard_lines + [cleared], depth - 1)
            best_score = max(score, best_score)

        return best_score

unique_rotation_table = {
    'T' : [0,1,2,3],
    'square' : [0],
    'L' : [0,1,2,3],
    'bL' : [0,1,2,3],
    'Z' : [0,1],
    'bZ' : [0,1],
    'line' : [0,1]
}


def legal_transforms(piece: Piece, grid_shape=(10, 24)) -> [(int, int)]:
    transforms = []
    for theta in unique_rotation_table[piece.type]:
        shape = np.rot90(piece.shape, k=theta)
        for x in range(grid_shape[0] + 1 - shape.shape[0]):
            transforms.append((x, theta))
    return transforms


def max_y_drop(grid: np.ndarray, piece: np.ndarray, dx: int) -> int:

    min_y = 24

    for i in range(piece.shape[0]):
        board_height = 24
        for y in range(grid.shape[1]):
            if grid[i + dx, y]:
                board_height = y
                break

        piece_height = 0
        for y in range(piece.shape[1]):
            if piece[i,-1-y]:
                piece_height = y
                break

        min_y = min(min_y, board_height + piece_height - piece.shape[1])
    return min_y
    #
    #
    # for gy in range(grid.shape[1] + 1):
    #     for px in range(piece.shape[0]):
    #         for py in range(piece.shape[1]):
    #             if gy + py >= grid.shape[1] or (piece[px, py] and grid[dx + px, gy + py]):
    #                 return gy - 1
    #
    # raise Exception("exceeded grid")


def drop_piece(grid: np.ndarray, piece: np.ndarray, transform: (int, int)) -> np.ndarray:
    piece = np.rot90(piece, k=transform[1])

    final_y = max_y_drop(grid, piece, transform[0])

    new_grid = grid.copy()
    for px in range(piece.shape[0]):
        for py in range(piece.shape[1]):
            new_grid[transform[0] + px, final_y + py] |= piece[px, py]

    return new_grid


def clear_lines(grid: np.ndarray) -> (np.ndarray, int):

    cleared_grid = np.zeros(grid.shape, bool)
    cleared = grid.all(axis=(0,))
    total_cleared = 0
    y = grid.shape[1] - 1

    for ty in range(grid.shape[1]):
        if not cleared[grid.shape[1] - 1 - ty]:
            cleared_grid[..., y] = grid[..., grid.shape[1] - 1 - ty]
            y -= 1
        else:
            total_cleared += 1

    if total_cleared > 4:
        raise Exception("illegal board")

    return cleared_grid, total_cleared


def apply_transform(board: Board, transform: (int,int,bool)) -> (Board, int):
    dropped = drop_piece(board.grid, board.hold_piece.shape if transform[2] else board.current_piece.shape, (transform[0], transform[1]))
    grid, cleared = clear_lines(dropped)
    hold = board.current_piece if transform[2] else board.hold_piece
    current_pieces = board.next_pieces[0]
    next_pieces = board.next_pieces[1:]
    return Board(grid, hold, next_pieces, current_pieces), cleared


def find_best_move(board: Board) -> (int, int, bool):
    ctransforms = legal_transforms(board.current_piece)
    stransforms = legal_transforms(board.hold_piece)
    best_move = None
    best_score = -math.inf
    for trans in ctransforms:
        b, cleared = apply_transform(board, (trans[0], trans[1], False))
        score = score_recur(b, [cleared], 2)
        if score > best_score:
            best_score = score
            best_move = (trans[0], trans[1], False)

    for trans in stransforms:
        b, cleared = apply_transform(board, (trans[0], trans[1], True))
        score = score_recur(b, [cleared], 2)
        if score > best_score:
            best_score = score
            best_move = (trans[0], trans[1], True)

    return best_move

