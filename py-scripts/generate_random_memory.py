import random
import sys

if len(sys.argv) < 2:
    print("specify the output file name!")

determ = False
if len(sys.argv) > 2:
    if sys.argv[2] == "determ":
        determ = True

random.seed(2333)


N = 4096
# N = 2048

# for k in range(10):
# fout = open(sys.argv[1]+".%d"%k, "wb")
fout = open(sys.argv[1], "wb")
for i in range(0, N):    
    if determ:
        contents = bytes([24 for i in range(0, 1024)])
    else:
        contents = bytes([random.randint(0, 255) if (i%8 != 7) else 0 for i in range(0, 1024)])
    # contents = bytes([24 for i in range(0, 1024)])
    fout.write(contents)
    print("\r%d/%d" % (i, N), end="")
fout.flush()
fout.close()
print()
    # print("%d/%d" % (k, 10))



print()
