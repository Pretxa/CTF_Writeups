import os
from PIL import Image

W = 128
H = 128

def choose_pos(idx, W, H):
    i = idx
    x = (i*73 + i*i*19 + 17) % W
    y = (i*131 + i*i*7 + 23) % H
    return x, y

def rot_for_idx(idx):
    i = idx
    return (i*7 + i*i*3 + 5) % 26

def rot_char(c, r):
    if 'A' <= c <= 'Z':
        return chr((ord(c)-ord('A')+r)%26 + ord('A'))
    if 'a' <= c <= 'z':
        return chr((ord(c)-ord('a')+r)%26 + ord('a'))
    return c

def unrot_char(c, r):
    return rot_char(c, 26 - r)


images = sorted(os.listdir("out/"))
flag = ""

for fname in images:
    idx = int(fname.split("_")[1].split(".")[0])
    img = Image.open(os.path.join("out/", fname))
    x, y = choose_pos(idx, W, H)
    rc = img.getpixel((x, y))[0]  # red channel
    flag += unrot_char(chr(rc), rot_for_idx(idx))

print("Flag:", flag)

