import PIL.Image
from numpy import ndarray
from Piece import Piece
from BoardImage import BoardImage


class Board:
    current_piece: Piece
    hold_piece: Piece
    grid: ndarray
    next_pieces: [Piece, Piece, Piece, Piece]

    @staticmethod
    def test_board():
        grid = ndarray((10, 24), dtype=bool)
        grid.fill(False)
        grid[1:3,22:24].fill(True)
        grid[3,22] = True
        grid[4,22] = True
        grid[4,23] = True
        grid[5,23] = True
        # for y in range(15, 20):
        #     for x in range(10):
        #         grid[x, y] = True
        #     grid[0, y] = False
        return Board(grid, Piece('bZ'), [Piece('Z'), Piece('T')], Piece('bL'))

    @staticmethod
    def from_image(img: BoardImage):
        grid = ndarray((10, 24), dtype=bool)
        for y in range(grid.shape[1]):
            for x in range(grid.shape[0]):
                stepX = img.playfield[2] / grid.shape[0]
                stepY = img.playfield[3] / grid.shape[1]
                xy = (int(img.playfield[0] + stepX / 2 + x * stepX), int(img.playfield[1] + stepY / 2 + y * stepY))
                pixel = img.image.getpixel(xy)
                grid[x, y] = (pixel[0]+pixel[1]+pixel[2] > 100)

        current_piece = None
        for y in range(grid.shape[1]):
            stepX = img.playfield[2] / grid.shape[0]
            stepY = img.playfield[3] / grid.shape[1]
            xy = (int(img.playfield[0] + stepX / 2 + 5 * stepX), int(img.playfield[1] + stepY / 2 + y * stepY))
            pixel = img.image.getpixel(xy)
            if pixel[0] > 20:
                current_piece = Piece.from_color(pixel)
                break

        if current_piece is None:
            raise Exception("couldn't find current piece")

        # exclude falling piece
        nonempty_rows = grid.any(axis=0)
        for ty in range(grid.shape[1]):
            y = grid.shape[1] - 1 - ty
            if not nonempty_rows[y]:
                grid[..., 0:y].fill(False)
                break

        hold_color = img.image.getpixel((img.hold[0] + 35, img.hold[1] + 29))
        hold = Piece.from_color(hold_color)

        next_pieces = []
        for p in range(5):
            stepY = img.next[3]/5
            startX = img.next[0] + img.next[2]/2
            startY = img.next[1] + stepY/2 + 8
            xy = (int(startX), int(startY + p * stepY))
            next_color = img.image.getpixel(xy)
            next_pieces.append(Piece.from_color(next_color))

        return Board(grid, hold, next_pieces, current_piece)

    def __init__(self, grid: ndarray, hold: Piece, next_pieces: [Piece, Piece, Piece, Piece], current_piece):
        self.grid = grid
        self.hold_piece = hold
        self.next_pieces = next_pieces
        self.current_piece = current_piece

    def encode(self):
        grid_str = ''
        for y in range(self.grid.shape[1]):
            for x in range(self.grid.shape[0]):
                grid_str += 'X' if self.grid[x,y] else '.'

        piece_str = ' '.join(map(lambda p : p.type, [self.current_piece, self.hold_piece] + self.next_pieces))
        command_str = f"{grid_str} {piece_str}\n"
        return command_str
