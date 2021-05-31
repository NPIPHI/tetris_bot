import numpy as np
from numpy import ndarray

piece_colors: [((int, int, int), str)] = [
    ((69, 193, 148), 'line'),
    ((170, 74, 160), 'T'),
    ((139, 187, 62), 'bZ'),
    ((188, 164, 65), 'square'),
    ((91, 74, 170), 'bL'),
    ((188, 110, 62), 'L'),
    ((189, 63, 70), 'Z'),
    ((177, 253, 51), 'bZ'),
    ((253, 213, 52), 'square'),
    ((253, 129, 52), 'L')
]

piece_shapes = {
    'line': np.asarray([[True, True, True, True]]),
    'T': np.asarray([[False, True, False],
                     [True, True, True]]),
    'bZ': np.asarray([[False, True, True],
                      [True, True, False]]),
    'square': np.asarray([[True, True],
                          [True, True]]),
    'bL': np.asarray([[True, False, False],
                      [True, True, True]]),
    'L': np.asarray([[False, False, True],
                     [True, True, True]]),
    'Z': np.asarray([[True, True, False],
                     [False, True, True]]),
}

for key in piece_shapes.keys():
    piece_shapes[key] = piece_shapes[key].transpose()


def distance(a: (int, int, int), b: (int, int, int)) -> int:
    dist = 0
    for (ai, bi) in zip(a, b):
        dist += abs(ai - bi)
    return dist


class Piece:
    type: str
    shape: ndarray

    @staticmethod
    def from_color(color: (int, int, int)):
        piece = min(map(lambda x: (distance(x[0], color), x[1]), piece_colors))[1]
        return Piece(piece)

    def __init__(self, piece_type: str):
        if piece_type not in ['L', 'bL', 'Z', 'bZ', 'T', 'line', 'square']:
            raise Exception("bad piece type")
        self.type = piece_type
        self.shape = piece_shapes[piece_type]