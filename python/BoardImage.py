import PIL
import pyautogui


class BoardImage:
    image: PIL.Image.Image
    global_offset: (int,int)
    playfield: (int,int,int,int)
    hold: (int,int,int,int)
    next: (int,int,int,int)
    whole: (int,int,int,int)

    def __init__(self):
        hold_pos = pyautogui.locateOnScreen("target.png", confidence=0.9)
        if hold_pos is None:
            raise Exception("failed to find board")

        self.global_offset = (hold_pos[0]-50, hold_pos[1] - 1 - 31*4)
        self.image = pyautogui.screenshot(region=(self.global_offset[0], self.global_offset[1], 500, 500))

        _, top = scan_to(self.image, (90, 0), (0, 1))
        left, _ = scan_to(self.image, (0, 150), (1,0))
        right, _ = scan_to(self.image, (387, 300), (-1,0))
        _, bottom = scan_to(self.image, (int((left + right)/2 - 50), 499), (0, -1))
        self.whole = (left, top, right-left, bottom-top)
        self.playfield = scan_rect(self.image, (self.whole[0] + int(self.whole[2]/2), self.whole[1] + 10), max_extent=self.whole)
        piece_width = self.playfield[2]/10
        self.playfield = (self.playfield[0], int(self.whole[1] - piece_width * 4), self.playfield[2], int(self.whole[3] - 3 + piece_width * 4))

        self.hold = scan_rect(self.image, (self.whole[0] + 50, self.whole[1] + 50))
        self.next = scan_rect(self.image, (self.whole[0] + self.whole[2] - 50, self.whole[1]+150))

        # outline_rect(self.image, self.whole, (0,255,0))
        # outline_rect(self.image, self.playfield, (255,0,0))
        # outline_rect(self.image, self.hold, (0,0,255))
        # outline_rect(self.image, self.next, (255,255,0))
        # self.image.save("i.png")

    def refresh(self):
        self.image = pyautogui.screenshot(region=(self.global_offset[0], self.global_offset[1], 500, 500))


def outline_rect(img: PIL.Image.Image, rect: (int, int, int, int), color: (int, int, int)):
    img.putpixel((rect[0], rect[1]), color)
    img.putpixel((rect[0] + rect[2], rect[1]), color)
    img.putpixel((rect[0] + rect[2], rect[1] + rect[3]), color)
    img.putpixel((rect[0], rect[1] + rect[3]), color)


def is_white(color: (int,int,int)):
    if color[0] * color[1] * color[2] == 0:
        return False
    max_ratio = max([color[a%3]/color[a//3] for a in range(9)])
    return color[0] > 100 and max_ratio < 1.4


def scan_to(img: PIL.Image.Image, start: (int,int), dir: (int,int)) -> (int,int):
    while start[0] >= 0 and start[1] >= 0 and start[0] < img.width and start[1] < img.height:
        pix = img.getpixel(start)
        if is_white(pix):
            return start

        start = (start[0] + dir[0], start[1] + dir[1])

    return (None,None)


def scan_rect(img: PIL.Image.Image, start: (int,int), max_extent=None) -> (int,int,int,int):
    _, top = scan_to(img, start, (0, -1))
    _, bottom = scan_to(img, start, (0, 1))
    left, _ = scan_to(img, start, (-1, 0))
    right, _ = scan_to(img, start, (1, 0))

    if max_extent is not None:
        if top is None:
            top = max_extent[1]
        if bottom is None:
            bottom = max_extent[1] + max_extent[3]
        if left is None:
            top = max_extent[0]
        if right is None:
            bottom = max_extent[0] + max_extent[2]

    return left, top, right - left, bottom - top


def distance(a: (int, int, int), b: (int, int, int)) -> int:
    dist = 0
    for (ai, bi) in zip(a, b):
        dist += abs(ai - bi)
    return dist