import time
from subprocess import PIPE, Popen

path = R"C:\Users\16182\PycharmProjects\tetris\cpp\cmake-build-release\findmove.exe"
test_str = R".........................................................................................................................................................................................X..X.XX..XX.XX.XX..XXXXX.XXXXXXXXX.XXXXXXXXX.XXXXXXXXX. bL bZ square L T bZ bL"
start = time.time()

for _ in range(10):
    child = Popen(f"{path} {test_str}",
                  stdin=PIPE, stdout=PIPE)
    result = b''
    while result == b'':
        result = child.stdout.readline().strip()
    try:
        (dx,dt,s,count) = [int(x) for x in result.split(b':')]
    except:
        print("unexpected result")
        print(result)

    child.kill()

end = time.time()
print(end - start)

# base: 0.72