import pygame
import subprocess
import os
import signal

ENGINE = "./main"
SOURCE = "main.c"
FRAMEFILE = "frame.bin"
INPUTFILE = "input.bin"

compile_cmd = [
    "cc",
    "-nostdlib",
    "-static",
    "-fno-stack-protector",
    "-fno-pie",
    "-no-pie",
    SOURCE,
    "-o", ENGINE
]

subprocess.run(compile_cmd, check=True)
engine_proc = subprocess.Popen([ENGINE])

pygame.init()
WIDTH, HEIGHT = 128, 128
SCALE = 4
screen = pygame.display.set_mode((WIDTH*SCALE, HEIGHT*SCALE))
clock = pygame.time.Clock()

def write_input():
    keys = pygame.key.get_pressed()
    up = 1 if keys[pygame.K_w] else 0
    down = 1 if keys[pygame.K_s] else 0
    left = 1 if keys[pygame.K_a] else 0
    right = 1 if keys[pygame.K_d] else 0
    with open(INPUTFILE, "wb") as f:
        f.write(bytes([up, down, left, right]))

def read_frame():
    if not os.path.exists(FRAMEFILE):
        return None
    try:
        with open(FRAMEFILE, "rb") as f:
            header = f.read(4)
            if len(header) < 4:
                return None
            w = (header[0] << 8) | header[1]
            h = (header[2] << 8) | header[3]
            rgb = f.read(w*h*3)
            if len(rgb) < w*h*3:
                return None
            return w, h, rgb
    except:
        return None

running = True
while running:
    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            running = False

    write_input()
    frame = read_frame()

    if frame:
        w, h, rgb = frame
        surf = pygame.Surface((w, h))
        idx = 0
        for y in range(h):
            for x in range(w):
                r = rgb[idx]; g = rgb[idx+1]; b = rgb[idx+2]
                idx += 3
                surf.set_at((x, y), (r, g, b))
        surf = pygame.transform.scale(surf, (WIDTH*SCALE, HEIGHT*SCALE))
        screen.blit(surf, (0, 0))

    pygame.display.flip()
    clock.tick(60)

pygame.quit()
engine_proc.send_signal(signal.SIGKILL)
