# importing pycairo
import cairo
import math
from random import random


def scale_color(color):
    return color / 255.0


def distance(x1, y1, x2, y2):
    return math.sqrt((x1 - x2) ** 2 + (y1 - y2) ** 2)


# Creating function for make roundrect shape
def roundrect(context, x, y, width, height, r):
    context.move_to(x, y)
    context.arc(x + r, y + r, r, math.pi, 3 * math.pi / 2)
    context.arc(x + width - r, y + r, r, 3 * math.pi / 2, 0)
    context.arc(x + width - r, y + height - r, r, 0, math.pi / 2)
    context.arc(x + r, y + height - r, r, math.pi / 2, math.pi)
    context.close_path()


def buil_unit(context, x, y, width, rad, r, g, b, a):
    roundrect(context, x, y, width, width, rad)
    context.set_source_rgba(scale_color(r), scale_color(g), scale_color(b), a)
    context.fill()


# Reference: https://color.adobe.com/Tron-Legacy-Film-Poster-color-theme-13490595/
palette = [
    # black
    (1, 31, 38, 1),
    # darker blue
    (23, 56, 64, 1),
    # dark blue
    (2, 104, 115, 1),
    # light blue
    (4, 191, 191, 1),
    # yellow
    (242, 203, 5, 1),
    # orange
    (242, 160, 7, 1),
    # red
    (156, 29, 44, 1),
]


# palette.reverse()
def get_color(_char):
    try:
        return palette[int(_char)]
    except:
        return (0, 0, 0, 0)


# music_str = []
# music_str.append("112   555 12    55  23455 34  2345")
# music_str.append("112   555 12    55 123455 34 12345")
# music_str.append("1123 4555 12    55 12        12   ")
# music_str.append("11 344 55 12    55 12345  34 12   ")
# music_str.append("11 344 55 12    55  23455 34 12   ")
# music_str.append("11  4  55 12334455     55 34 12   ")
# music_str.append("11     55 12334455 123455 34 12345")
# music_str.append("11     55  233445  12345  34  2345")

music_str = []
music_str.append("555   555 55    55  55555 45  5555")
music_str.append("555   555 55    55 555555 32 55555")
music_str.append("4444 4444 44    44 44        44   ")
music_str.append("44 444 44 44    44 44444  44 44   ")
music_str.append("33 333 33 33    33  33333 33 33   ")
music_str.append("33  3  33 33333333     33 33 33   ")
music_str.append("22     22 22222222 222222 22 22222")
music_str.append("11     11  111111  11111  11  1111")

line_length = len(music_str[0])
for line in music_str:
    if len(line) != line_length:
        print("ERROR: Different line lengths ...")
        exit(666)


ROOM = 10
BOX_WIDTH = 30
INNER_BOX_ROOM = 5
NX_BOXES = len(music_str[0])
NY_BOXES = len(music_str)
WIDTH = NX_BOXES * (BOX_WIDTH + INNER_BOX_ROOM) + 2 * ROOM
HEIGHT = NY_BOXES * (BOX_WIDTH + INNER_BOX_ROOM) + 2 * ROOM


def print_layer(context, layer):
    # Set a background color
    # if first_context:
    #     pass
    #     context.set_source_rgb(scale_color(1), scale_color(31), scale_color(38))
    #     context.paint()

    for j, col in enumerate(layer):
        for i, row in enumerate(layer[j]):
            unit_x = ROOM + i * (BOX_WIDTH + INNER_BOX_ROOM)
            unit_y = ROOM + j * (BOX_WIDTH + INNER_BOX_ROOM)
            # dist = (
            #     distance(
            #         EPICENTRE_X + BOX_WIDTH / 2,
            #         EPICENTRE_Y + BOX_WIDTH / 2,
            #         unit_x,
            #         unit_y,
            #     )
            #     / max_distance
            # )
            # color_index = min(6, math.floor((1 - math.exp(-4 * dist)) * len(palette)))
            color = get_color(layer[j][i])
            # print(f"color_index: {color_index}")
            buil_unit(
                context,
                unit_x,
                unit_y,
                BOX_WIDTH,
                3,
                color[0],
                color[1],
                color[2],
                color[3],
            )

    # setting scale of the context
    context.scale(WIDTH, HEIGHT)
    context.save()


def main():
    print("Building image ...")

    with cairo.SVGSurface("music_t.svg", WIDTH, HEIGHT) as surface:
        # background
        # context = cairo.Context(surface)
        # roundrect(context, 0, 0, WIDTH, HEIGHT, 10)
        # context.set_source_rgba(
        #     scale_color(1),
        #     scale_color(31),
        #     scale_color(38),
        #     1,
        # )
        # context.fill()
        # context.scale(WIDTH, HEIGHT)
        # context.save()

        # foreground
        context = cairo.Context(surface)
        print_layer(context, music_str)


if __name__ == "__main__":
    main()
