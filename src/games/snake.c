#include "snake.h"
#include "vga13h.h"
#include "keyboard.h"
#include "timer.h"
#include <stdint.h>

#define CELL        8
#define COLS        38
#define ROWS        23
#define OFS_X       8
#define OFS_Y       8
#define MAX_LEN     (COLS * ROWS)

#define C_BG        0
#define C_BORDER    8
#define C_SNAKE_H   10
#define C_SNAKE_B   2
#define C_FOOD      12
#define C_TEXT      15
#define C_SCORE_V   14
#define C_GAMEOVER  12

#define DIR_UP      0
#define DIR_DOWN    1
#define DIR_LEFT    2
#define DIR_RIGHT   3

#define SC_W        0x11
#define SC_A        0x1E
#define SC_S        0x1F
#define SC_D        0x20
#define SC_Q        0x10
#define SC_ESC      0x01
#define SC_UP       0x48
#define SC_DOWN     0x50
#define SC_LEFT     0x4B
#define SC_RIGHT    0x4D
#define SC_CTRL     0x1D
#define SC_CTRL_REL 0x9D
#define SC_C        0x2E

static uint32_t rng_state = 0xDEADBEEF;

static uint32_t rng_next(void)
{
    rng_state = rng_state * 1664525u + 1013904223u;
    return rng_state;
}

static void fill_cell(int cx, int cy, uint8_t color)
{
    int px = OFS_X + cx * CELL;
    int py = OFS_Y + cy * CELL;
    for (int dy = 0; dy < CELL; dy++)
        for (int dx = 0; dx < CELL; dx++)
            vga13h_putpixel(px + dx, py + dy, color);
}

static void draw_border(void)
{
    for (int x = 0; x < VGA13H_WIDTH; x++) {
        for (int t = 0; t < OFS_Y; t++) {
            vga13h_putpixel(x, t, C_BORDER);
            vga13h_putpixel(x, VGA13H_HEIGHT - 1 - t, C_BORDER);
        }
    }
    for (int y = 0; y < VGA13H_HEIGHT; y++) {
        for (int t = 0; t < OFS_X; t++) {
            vga13h_putpixel(t, y, C_BORDER);
            vga13h_putpixel(VGA13H_WIDTH - 1 - t, y, C_BORDER);
        }
    }
}

static void draw_number(int x, int y, int val, uint8_t fg, uint8_t bg)
{
    char buf[12];
    int n = 0;
    if (val == 0) {
        buf[n++] = '0';
    } else {
        int tmp = val;
        while (tmp > 0) { buf[n++] = '0' + (tmp % 10); tmp /= 10; }
        for (int l = 0, r = n - 1; l < r; l++, r--) {
            char t = buf[l]; buf[l] = buf[r]; buf[r] = t;
        }
    }
    for (int i = 0; i < n; i++)
        vga13h_putchar(x + i * VGA13H_FONT_W, y, buf[i], fg, bg);
}

static void draw_string(int x, int y, const char *s, uint8_t fg, uint8_t bg)
{
    for (int i = 0; s[i]; i++)
        vga13h_putchar(x + i * VGA13H_FONT_W, y, s[i], fg, bg);
}

static void draw_hud(int score, int length)
{
    for (int x = OFS_X; x < VGA13H_WIDTH - OFS_X; x++)
        for (int y = 1; y < OFS_Y; y++)
            vga13h_putpixel(x, y, C_BG);

    draw_string(OFS_X,      1, "SCORE:", C_TEXT,    C_BG);
    draw_number(OFS_X + 48, 1, score,   C_SCORE_V, C_BG);
    draw_string(160,        1, "LEN:",  C_TEXT,    C_BG);
    draw_number(192,        1, length,  C_SCORE_V, C_BG);
}

static void place_food(int sx[], int sy[], int slen, int *fx, int *fy)
{
    for (;;) {
        int cx = (int)(rng_next() % (uint32_t)COLS);
        int cy = (int)(rng_next() % (uint32_t)ROWS);
        int hit = 0;
        for (int i = 0; i < slen; i++)
            if (sx[i] == cx && sy[i] == cy) { hit = 1; break; }
        if (!hit) { *fx = cx; *fy = cy; return; }
    }
}

static void game_over_screen(int score)
{
    for (int y = OFS_Y; y < VGA13H_HEIGHT - OFS_Y; y += 2)
        for (int x = OFS_X; x < VGA13H_WIDTH - OFS_X; x += 2)
            vga13h_putpixel(x, y, C_BG);

    draw_string(88,  84, "GAME OVER", C_GAMEOVER, C_BG);
    draw_string(72,  96, "SCORE:",    C_TEXT,     C_BG);
    draw_number(120, 96, score,       C_SCORE_V,  C_BG);
    draw_string(60, 108, "PRESS  Q",  C_TEXT,     C_BG);

    for (;;) {
        while (!keyboard_data_available())
            __asm__ volatile("hlt");
        unsigned char sc = keyboard_read_raw();
        if (sc == SC_Q || sc == SC_ESC)
            return;
    }
}

void snake_run(void)
{
    static int sx[MAX_LEN], sy[MAX_LEN];
    int slen, dir, food_x, food_y, score;
    int gameover;
    int ctrl_held = 0;

    rng_state ^= timer_get_ticks();

    vga13h_clear(C_BG);
    draw_border();

    slen     = 3;
    dir      = DIR_RIGHT;
    score    = 0;
    gameover = 0;

    sx[0] = COLS / 2;     sy[0] = ROWS / 2;
    sx[1] = COLS / 2 - 1; sy[1] = ROWS / 2;
    sx[2] = COLS / 2 - 2; sy[2] = ROWS / 2;

    fill_cell(sx[0], sy[0], C_SNAKE_H);
    for (int i = 1; i < slen; i++)
        fill_cell(sx[i], sy[i], C_SNAKE_B);

    place_food(sx, sy, slen, &food_x, &food_y);
    fill_cell(food_x, food_y, C_FOOD);
    draw_hud(score, slen);

    uint32_t last_tick = timer_get_ticks();
    uint32_t speed = 10;

    while (!gameover) {

        while (keyboard_data_available()) {
            unsigned char sc = keyboard_read_raw();

            if (sc == SC_CTRL)     { ctrl_held = 1; continue; }
            if (sc == SC_CTRL_REL) { ctrl_held = 0; continue; }
            if (ctrl_held && sc == SC_C) { gameover = 2; break; }

            if (sc & 0x80) continue;

            switch (sc) {
            case SC_W: case SC_UP:
                if (dir != DIR_DOWN)  dir = DIR_UP;
                break;
            case SC_S: case SC_DOWN:
                if (dir != DIR_UP)    dir = DIR_DOWN;
                break;
            case SC_A: case SC_LEFT:
                if (dir != DIR_RIGHT) dir = DIR_LEFT;
                break;
            case SC_D: case SC_RIGHT:
                if (dir != DIR_LEFT)  dir = DIR_RIGHT;
                break;
            case SC_Q: case SC_ESC:
                gameover = 2;
                break;
            }
        }

        if (gameover) break;

        uint32_t now = timer_get_ticks();
        if (now - last_tick < speed)
            continue;
        last_tick = now;

        int nx = sx[0], ny = sy[0];
        switch (dir) {
        case DIR_UP:    ny--; break;
        case DIR_DOWN:  ny++; break;
        case DIR_LEFT:  nx--; break;
        case DIR_RIGHT: nx++; break;
        }

        if (nx < 0 || nx >= COLS || ny < 0 || ny >= ROWS) {
            gameover = 1;
            break;
        }

        for (int i = 0; i < slen - 1; i++) {
            if (sx[i] == nx && sy[i] == ny) {
                gameover = 1;
                break;
            }
        }
        if (gameover) break;

        int ate = (nx == food_x && ny == food_y);

        if (ate) {
            score += 10;
            slen++;
            if (slen >= MAX_LEN) slen = MAX_LEN;
            speed = 10 - (uint32_t)(score / 50);
            if (speed < 3) speed = 3;
        }

        if (!ate)
            fill_cell(sx[slen - 1], sy[slen - 1], C_BG);

        for (int i = slen - 1; i > 0; i--) {
            sx[i] = sx[i - 1];
            sy[i] = sy[i - 1];
        }

        fill_cell(sx[1], sy[1], C_SNAKE_B);

        sx[0] = nx;
        sy[0] = ny;
        fill_cell(sx[0], sy[0], C_SNAKE_H);

        if (ate) {
            place_food(sx, sy, slen, &food_x, &food_y);
            fill_cell(food_x, food_y, C_FOOD);
            draw_hud(score, slen);
        }
    }

    if (gameover == 1)
        game_over_screen(score);

    vga13h_clear(C_BG);
    vga13h_set_cursor(0, 0);
    vga13h_set_fg(C_TEXT);
    vga13h_set_bg(C_BG);
}
