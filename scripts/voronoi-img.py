# This file copied from https://rosettacode.org/wiki/Voronoi_diagram

from PIL import Image
import random
import math
import sys

PALETTE = [
    (34,  139, 34),
    (187, 109, 62),
    (165, 42, 42),
    (184, 134, 11),
    (255, 215, 0),
    (50, 205,  50) #grass green (sheepies are generated on these tiles)
]            

    #( 97,  63, 16)  #dirt brown  (houses are generated on these tiles)
def generate_voronoi_diagram(path, width, height, num_cells):
    image = Image.new("RGB", (width, height))
    putpixel = image.putpixel
    imgx, imgy = image.size
    nx = []
    ny = []
    ncolor = []
    for i in range(num_cells):
        nx.append(random.randrange(imgx))
        ny.append(random.randrange(imgy))
        ncolor.append(random.choice(PALETTE))
    for y in range(imgy):
        for x in range(imgx):
            dmin = math.hypot(imgx-1, imgy-1)
            j = -1
            for i in range(num_cells):
                d = math.hypot(nx[i]-x, ny[i]-y)
                if d < dmin:
                    dmin = d
                    j = i
            putpixel((x, y), ncolor[j])
    image.save(path, "PNG")

if __name__ == "__main__":
    if len(sys.argv) != 5:
        print('Usage: python %s <width> <height> <number of voronoi cells> <output image file>' % sys.argv[0])
        sys.exit(1)

    width, height, num_cells = map(int, sys.argv[1:4])
    path = sys.argv[4]       
    generate_voronoi_diagram(path, width, height, num_cells)
