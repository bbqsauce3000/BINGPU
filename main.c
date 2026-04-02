typedef unsigned long u64;
typedef unsigned char u8;
typedef long s64;

#define WIDTH 128
#define HEIGHT 128

static s64 sys_write(int fd, const void *buf, u64 len) {
    s64 ret;
    __asm__ volatile("mov $1, %%rax; syscall"
        : "=a"(ret)
        : "D"(fd), "S"(buf), "d"(len)
        : "rcx","r11","memory");
    return ret;
}

static s64 sys_read(int fd, void *buf, u64 len) {
    s64 ret;
    __asm__ volatile("mov $0, %%rax; syscall"
        : "=a"(ret)
        : "D"(fd), "S"(buf), "d"(len)
        : "rcx","r11","memory");
    return ret;
}

static s64 sys_open(const char *path, int flags, int mode) {
    s64 ret;
    __asm__ volatile("mov $2, %%rax; syscall"
        : "=a"(ret)
        : "D"(path), "S"(flags), "d"(mode)
        : "rcx","r11","memory");
    return ret;
}

static s64 sys_close(int fd) {
    s64 ret;
    __asm__ volatile("mov $3, %%rax; syscall"
        : "=a"(ret)
        : "D"(fd)
        : "rcx","r11","memory");
    return ret;
}

static s64 sys_nanosleep(const void *req, void *rem) {
    s64 ret;
    __asm__ volatile("mov $35, %%rax; syscall"
        : "=a"(ret)
        : "D"(req), "S"(rem)
        : "rcx","r11","memory");
    return ret;
}

static void sys_exit(int code) {
    __asm__ volatile("mov $60, %%rax; syscall"
        :
        : "D"(code)
        : "rcx","r11","memory");
    while (1) {}
}

#define O_CREAT 0100
#define O_TRUNC 01000
#define O_WRONLY 01

struct timespec { s64 tv_sec; s64 tv_nsec; };

static void sleep_ms(int ms) {
    struct timespec ts;
    ts.tv_sec = 0;
    ts.tv_nsec = (s64)ms * 1000000;
    sys_nanosleep(&ts, 0);
}

static u8 fb[WIDTH * HEIGHT * 3];

static void clear_fb() {
    for (int i = 0; i < WIDTH * HEIGHT * 3; i++)
        fb[i] = 0;
}

static void put_pixel(int x, int y, u8 r, u8 g, u8 b) {
    if (x < 0 || y < 0 || x >= WIDTH || y >= HEIGHT) return;
    int idx = (y * WIDTH + x) * 3;
    fb[idx] = r; fb[idx+1] = g; fb[idx+2] = b;
}

static void draw_square(int cx, int cy) {
    for (int y = -8; y <= 8; y++)
        for (int x = -8; x <= 8; x++)
            put_pixel(cx+x, cy+y, 255, 255, 0);
}

static void write_frame() {
    int fd = (int)sys_open("frame.bin", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) return;
    u8 header[4] = {
        (WIDTH >> 8) & 0xFF, WIDTH & 0xFF,
        (HEIGHT >> 8) & 0xFF, HEIGHT & 0xFF
    };
    sys_write(fd, header, 4);
    sys_write(fd, fb, WIDTH * HEIGHT * 3);
    sys_close(fd);
}

void _start() {
    int x = 64;
    int y = 64;

    while (1) {
        u8 input[4] = {0,0,0,0};
        int fd_in = (int)sys_open("input.bin", 0, 0);
        if (fd_in >= 0) {
            sys_read(fd_in, input, 4);
            sys_close(fd_in);
        }

        if (input[0]) y -= 2;
        if (input[1]) y += 2;
        if (input[2]) x -= 2;
        if (input[3]) x += 2;

        if (x < 8) x = 8;
        if (y < 8) y = 8;
        if (x > WIDTH-8) x = WIDTH-8;
        if (y > HEIGHT-8) y = HEIGHT-8;

        clear_fb();
        draw_square(x, y);
        write_frame();
        sleep_ms(16);
    }

    sys_exit(0);
}
