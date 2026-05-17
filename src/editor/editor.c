#include "editor.h"
#include "vga.h"
#include "io.h"
#include "fs.h"
#include "kheap.h"

static volatile uint16_t *vga_buf = (volatile uint16_t *)VGA_MEMORY;

#define VGA_ENTRY(c, color) ((uint16_t)(unsigned char)(c) | ((uint16_t)(color) << 8))
#define ST_COLOR (VGA_COLOR_WHITE | (VGA_COLOR_BLUE << 4))
#define TXT_COLOR (VGA_COLOR_WHITE | (VGA_COLOR_BLACK << 4))
#define ST_ROW 0
#define TX_ROW 1
#define TX_HEIGHT (VGA_HEIGHT - 1)

<<<<<<< HEAD
#define SC_CTRL   0x1D
#define SC_LSHIFT 0x2A
#define SC_RSHIFT 0x36
#define SC_E0     0xE0
=======
#define SC_CTRL 0x1D
#define SC_LSHIFT 0x2A
#define SC_RSHIFT 0x36
#define SC_E0 0xE0
>>>>>>> f7fec4a (added new things and bug fixes)

static int ctrl_pressed = 0;
static int shift_pressed = 0;

static const char sc_map[128] = {
<<<<<<< HEAD
    0, 27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b', '\t',
    'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', 0,
    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0, '\\',
    'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' ',
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    '7', '8', '9', '-', '4', '5', '6', '+', '1', '2', '3', '0', '.', 0, 0,
};

typedef enum {
    EKEY_CHAR, EKEY_UP, EKEY_DOWN, EKEY_LEFT, EKEY_RIGHT,
    EKEY_HOME, EKEY_END, EKEY_PGUP, EKEY_PGDN,
    EKEY_DEL, EKEY_BACKSPACE, EKEY_ENTER, EKEY_TAB,
    EKEY_CTRL_S, EKEY_CTRL_Q, EKEY_CTRL_C, EKEY_CTRL_V,
    EKEY_NONE,
} EKeyType;

typedef struct {
=======
    0,
    27,
    '1',
    '2',
    '3',
    '4',
    '5',
    '6',
    '7',
    '8',
    '9',
    '0',
    '-',
    '=',
    '\b',
    '\t',
    'q',
    'w',
    'e',
    'r',
    't',
    'y',
    'u',
    'i',
    'o',
    'p',
    '[',
    ']',
    '\n',
    0,
    'a',
    's',
    'd',
    'f',
    'g',
    'h',
    'j',
    'k',
    'l',
    ';',
    '\'',
    '`',
    0,
    '\\',
    'z',
    'x',
    'c',
    'v',
    'b',
    'n',
    'm',
    ',',
    '.',
    '/',
    0,
    '*',
    0,
    ' ',
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    '7',
    '8',
    '9',
    '-',
    '4',
    '5',
    '6',
    '+',
    '1',
    '2',
    '3',
    '0',
    '.',
    0,
    0,
};

typedef enum
{
    EKEY_CHAR,
    EKEY_UP,
    EKEY_DOWN,
    EKEY_LEFT,
    EKEY_RIGHT,
    EKEY_HOME,
    EKEY_END,
    EKEY_PGUP,
    EKEY_PGDN,
    EKEY_DEL,
    EKEY_BACKSPACE,
    EKEY_ENTER,
    EKEY_TAB,
    EKEY_CTRL_O,
    EKEY_CTRL_X,
    EKEY_CTRL_C,
    EKEY_CTRL_V,
    EKEY_NONE,
} EKeyType;

typedef struct
{
>>>>>>> f7fec4a (added new things and bug fixes)
    EKeyType type;
    char ch;
} EKey;

<<<<<<< HEAD
static EKey make_key(EKeyType t, char c) {
=======
static EKey make_key(EKeyType t, char c)
{
>>>>>>> f7fec4a (added new things and bug fixes)
    EKey k;
    k.type = t;
    k.ch = c;
    return k;
}

<<<<<<< HEAD
static int gb_is_full(GapBuffer *gb) {
    return gb->gap_start >= gb->gap_end;
}

static int gb_logical_size(GapBuffer *gb) {
    return gb->gap_start + (gb->size - gb->gap_end);
}

static void gb_move_to(GapBuffer *gb, int pos) {
    if (pos < 0) pos = 0;
    int log_size = gb_logical_size(gb);
    if (pos > log_size) pos = log_size;

    if (pos < gb->gap_start) {
=======
static int gb_is_full(GapBuffer *gb)
{
    return gb->gap_start >= gb->gap_end;
}

static int gb_logical_size(GapBuffer *gb)
{
    return gb->gap_start + (gb->size - gb->gap_end);
}

static void gb_move_to(GapBuffer *gb, int pos)
{
    if (pos < 0)
        pos = 0;
    int log_size = gb_logical_size(gb);
    if (pos > log_size)
        pos = log_size;

    if (pos < gb->gap_start)
    {
>>>>>>> f7fec4a (added new things and bug fixes)
        int count = gb->gap_start - pos;
        int i;
        for (i = 0; i < count; i++)
            gb->buf[gb->gap_end - count + i] = gb->buf[pos + i];
        gb->gap_start = pos;
        gb->gap_end -= count;
<<<<<<< HEAD
    } else if (pos > gb->gap_start) {
=======
    }
    else if (pos > gb->gap_start)
    {
>>>>>>> f7fec4a (added new things and bug fixes)
        int count = pos - gb->gap_start;
        int i;
        for (i = 0; i < count; i++)
            gb->buf[gb->gap_start + i] = gb->buf[gb->gap_end + i];
        gb->gap_start = pos;
        gb->gap_end += count;
    }
}

<<<<<<< HEAD
static void update_cursor_pos(GapBuffer *gb) {
    int row = 0, col = 0;
    for (int i = 0; i < gb->gap_start; i++) {
        if (gb->buf[i] == '\n') {
            row++;
            col = 0;
        } else {
=======
static void update_cursor_pos(GapBuffer *gb)
{
    int row = 0, col = 0;
    for (int i = 0; i < gb->gap_start; i++)
    {
        if (gb->buf[i] == '\n')
        {
            row++;
            col = 0;
        }
        else
        {
>>>>>>> f7fec4a (added new things and bug fixes)
            col++;
        }
    }
    gb->cursor_row = row;
    gb->cursor_col = col;
}

<<<<<<< HEAD
static void gb_insert(GapBuffer *gb, char c) {
    if (gb_is_full(gb)) return;
=======
static void gb_insert(GapBuffer *gb, char c)
{
    if (gb_is_full(gb))
        return;
>>>>>>> f7fec4a (added new things and bug fixes)
    gb->buf[gb->gap_start] = c;
    gb->gap_start++;
    gb->modified = 1;
}

<<<<<<< HEAD
static void gb_newline(GapBuffer *gb) {
    gb_insert(gb, '\n');
}

static void gb_delete_back(GapBuffer *gb) {
    if (gb->gap_start == 0) return;
=======
static void gb_newline(GapBuffer *gb)
{
    gb_insert(gb, '\n');
}

static void gb_delete_back(GapBuffer *gb)
{
    if (gb->gap_start == 0)
        return;
>>>>>>> f7fec4a (added new things and bug fixes)
    gb->gap_start--;
    gb->modified = 1;
}

<<<<<<< HEAD
static void gb_delete_forward(GapBuffer *gb) {
    if (gb->gap_end >= gb->size) return;
=======
static void gb_delete_forward(GapBuffer *gb)
{
    if (gb->gap_end >= gb->size)
        return;
>>>>>>> f7fec4a (added new things and bug fixes)
    gb->gap_end++;
    gb->modified = 1;
}

<<<<<<< HEAD
static void gb_move_left(GapBuffer *gb) {
    if (gb->gap_start == 0) return;
    gb_move_to(gb, gb->gap_start - 1);
}

static void gb_move_right(GapBuffer *gb) {
    if (gb->gap_end >= gb->size) return;
    gb_move_to(gb, gb->gap_start + 1);
}

static void gb_home(GapBuffer *gb) {
=======
static void gb_move_left(GapBuffer *gb)
{
    if (gb->gap_start == 0)
        return;
    gb_move_to(gb, gb->gap_start - 1);
}

static void gb_move_right(GapBuffer *gb)
{
    if (gb->gap_end >= gb->size)
        return;
    gb_move_to(gb, gb->gap_start + 1);
}

static void gb_home(GapBuffer *gb)
{
>>>>>>> f7fec4a (added new things and bug fixes)
    int pos = gb->gap_start;
    while (pos > 0 && gb->buf[pos - 1] != '\n')
        pos--;
    gb_move_to(gb, pos);
}

<<<<<<< HEAD
static void gb_end(GapBuffer *gb) {
=======
static void gb_end(GapBuffer *gb)
{
>>>>>>> f7fec4a (added new things and bug fixes)
    int pos = gb->gap_end;
    while (pos < gb->size && gb->buf[pos] != '\n')
        pos++;
    int log_pos = gb->gap_start + (pos - gb->gap_end);
    gb_move_to(gb, log_pos);
}

<<<<<<< HEAD
static void gb_move_up(GapBuffer *gb) {
=======
static void gb_move_up(GapBuffer *gb)
{
>>>>>>> f7fec4a (added new things and bug fixes)
    update_cursor_pos(gb);
    int target_col = gb->cursor_col;

    int cur_line_start = gb->gap_start;
    while (cur_line_start > 0 && gb->buf[cur_line_start - 1] != '\n')
        cur_line_start--;

<<<<<<< HEAD
    if (cur_line_start == 0) return;

    int prev_line_end = cur_line_start - 2;
    if (prev_line_end < 0) {
=======
    if (cur_line_start == 0)
        return;

    int prev_line_end = cur_line_start - 2;
    if (prev_line_end < 0)
    {
>>>>>>> f7fec4a (added new things and bug fixes)
        gb_move_to(gb, 0);
        return;
    }

    int prev_line_start = prev_line_end;
    while (prev_line_start >= 0 && gb->buf[prev_line_start] != '\n')
        prev_line_start--;
    prev_line_start++;

    int prev_len = prev_line_end - prev_line_start + 1;
    if (target_col > prev_len)
        target_col = prev_len;

    gb_move_to(gb, prev_line_start + target_col);
}

<<<<<<< HEAD
static void gb_move_down(GapBuffer *gb) {
=======
static void gb_move_down(GapBuffer *gb)
{
>>>>>>> f7fec4a (added new things and bug fixes)
    update_cursor_pos(gb);
    int target_col = gb->cursor_col;

    int cur_line_end = gb->gap_end;
    while (cur_line_end < gb->size && gb->buf[cur_line_end] != '\n')
        cur_line_end++;

<<<<<<< HEAD
    if (cur_line_end >= gb->size) return;

    int next_line_start = cur_line_end + 1;
    if (next_line_start >= gb->size) return;
=======
    if (cur_line_end >= gb->size)
        return;

    int next_line_start = cur_line_end + 1;
    if (next_line_start >= gb->size)
        return;
>>>>>>> f7fec4a (added new things and bug fixes)

    int next_line_end = next_line_start;
    while (next_line_end < gb->size && gb->buf[next_line_end] != '\n')
        next_line_end++;

    int next_len = next_line_end - next_line_start;
    if (target_col > next_len)
        target_col = next_len;

    int log_pos = gb->gap_start + (next_line_start - gb->gap_end) + target_col;
    gb_move_to(gb, log_pos);
}

<<<<<<< HEAD
static void editor_save(GapBuffer *gb) {
    if (gb->filename[0] == '\0') return;
    int content_len = gb_logical_size(gb);
    if (content_len > FS_SIZE) content_len = FS_SIZE;
=======
static void editor_save(GapBuffer *gb)
{
    if (gb->filename[0] == '\0')
        return;
    int content_len = gb_logical_size(gb);
    if (content_len > FS_SIZE)
        content_len = FS_SIZE;
>>>>>>> f7fec4a (added new things and bug fixes)

    char tmp[FS_SIZE + 1];
    int pos = 0;
    for (int i = 0; i < gb->gap_start && pos < FS_SIZE; i++)
        tmp[pos++] = gb->buf[i];
    for (int i = gb->gap_end; i < gb->size && pos < FS_SIZE; i++)
        tmp[pos++] = gb->buf[i];
    tmp[pos] = '\0';

    if (!fs_exists(gb->filename))
        fs_create(gb->filename);
    fs_write(gb->filename, tmp, content_len);
    gb->modified = 0;
}

<<<<<<< HEAD
static void draw_status_bar(GapBuffer *gb) {
    char buf[81];
    int p = 0;

    buf[p++] = ' ';
    if (gb->filename[0]) {
        for (int i = 0; gb->filename[i] && p < 20; i++)
            buf[p++] = gb->filename[i];
    } else {
=======
static void draw_combo(char *buf, int *p, char key, const char *label)
{
    buf[(*p)++] = '[';
    buf[(*p)++] = '^';
    buf[(*p)++] = key;
    buf[(*p)++] = ']';
    while (*label)
        buf[(*p)++] = *label++;
    buf[(*p)++] = ' ';
    buf[(*p)++] = ' ';
}

static void draw_status_bar(GapBuffer *gb)
{
    char buf[81];
    int p = 0;

    draw_combo(buf, &p, 'O', "save");
    draw_combo(buf, &p, 'X', "quit");
    draw_combo(buf, &p, 'K', "cut");
    draw_combo(buf, &p, 'U', "paste");
    draw_combo(buf, &p, 'C', "copy");
    draw_combo(buf, &p, 'V', "paste");

    while (p < 58)
        buf[p++] = ' ';

    if (gb->filename[0])
    {
        for (int i = 0; gb->filename[i] && p < 74; i++)
            buf[p++] = gb->filename[i];
    }
    else
    {
>>>>>>> f7fec4a (added new things and bug fixes)
        buf[p++] = 'u';
        buf[p++] = 'n';
        buf[p++] = 'n';
        buf[p++] = 'a';
        buf[p++] = 'm';
        buf[p++] = 'e';
        buf[p++] = 'd';
    }

<<<<<<< HEAD
    while (p < 22) buf[p++] = ' ';
    buf[p++] = '[';
    if (gb->modified) {
        buf[p++] = '*';
        buf[p++] = 'M';
        buf[p++] = 'O';
        buf[p++] = 'D';
    } else {
        buf[p++] = ' ';
        buf[p++] = ' ';
        buf[p++] = ' ';
        buf[p++] = ' ';
    }
    buf[p++] = ']';

    while (p < 36) buf[p++] = ' ';
    buf[p++] = ' ';
=======
    buf[p++] = ' ';
    buf[p++] = '[';
    buf[p++] = gb->modified ? '*' : ' ';
    buf[p++] = ']';
    buf[p++] = ' ';

>>>>>>> f7fec4a (added new things and bug fixes)
    update_cursor_pos(gb);
    int r = gb->cursor_row;
    int c = gb->cursor_col;
    if (r >= 100) { buf[p++] = '0' + (r / 100); r %= 100; }
    if (r >= 10)  { buf[p++] = '0' + (r / 10); r %= 10; }
    buf[p++] = '0' + r;
    buf[p++] = ':';
    if (c >= 100) { buf[p++] = '0' + (c / 100); c %= 100; }
    if (c >= 10)  { buf[p++] = '0' + (c / 10); c %= 10; }
    buf[p++] = '0' + c;

<<<<<<< HEAD
    while (p < 54) buf[p++] = ' ';
    buf[p++] = '[';
    buf[p++] = '^';
    buf[p++] = 'S';
    buf[p++] = ']';
    buf[p++] = 's';
    buf[p++] = 'a';
    buf[p++] = 'v';
    buf[p++] = 'e';
    buf[p++] = ' ';
    buf[p++] = '[';
    buf[p++] = '^';
    buf[p++] = 'Q';
    buf[p++] = ']';
    buf[p++] = 'q';
    buf[p++] = 'u';
    buf[p++] = 'i';
    buf[p++] = 't';

    while (p < 80) buf[p++] = ' ';
=======
    while (p < 80)
        buf[p++] = ' ';
>>>>>>> f7fec4a (added new things and bug fixes)

    uint8_t color = ST_COLOR;
    for (int x = 0; x < VGA_WIDTH; x++)
        vga_buf[x] = VGA_ENTRY(buf[x], color);
}

<<<<<<< HEAD
static void render(GapBuffer *gb) {
=======
static void render(GapBuffer *gb)
{
>>>>>>> f7fec4a (added new things and bug fixes)
    for (int y = 0; y < VGA_HEIGHT; y++)
        for (int x = 0; x < VGA_WIDTH; x++)
            vga_buf[y * VGA_WIDTH + x] = VGA_ENTRY(' ', TXT_COLOR);

    draw_status_bar(gb);

    int row = 0, col = 0;
    int cur_row = 0, cur_col = 0;
    int at_cursor = 0;

<<<<<<< HEAD
    for (int i = 0; i < gb->size; i++) {
        if (i == gb->gap_start) {
=======
    for (int i = 0; i < gb->size; i++)
    {
        if (i == gb->gap_start)
        {
>>>>>>> f7fec4a (added new things and bug fixes)
            at_cursor = 1;
            cur_row = row;
            cur_col = col;
            i = gb->gap_end;
<<<<<<< HEAD
            if (i >= gb->size) break;
        }
        if (row >= TX_HEIGHT) break;
        if (gb->buf[i] == '\n') {
            row++;
            col = 0;
        } else {
=======
            if (i >= gb->size)
                break;
        }
        if (row >= TX_HEIGHT)
            break;
        if (gb->buf[i] == '\n')
        {
            row++;
            col = 0;
        }
        else
        {
>>>>>>> f7fec4a (added new things and bug fixes)
            if (col < VGA_WIDTH)
                vga_buf[(row + TX_ROW) * VGA_WIDTH + col] = VGA_ENTRY(gb->buf[i], TXT_COLOR);
            col++;
        }
    }

<<<<<<< HEAD
    if (!at_cursor) {
=======
    if (!at_cursor)
    {
>>>>>>> f7fec4a (added new things and bug fixes)
        cur_row = row;
        cur_col = col;
    }

    gb->cursor_row = cur_row;
    gb->cursor_col = cur_col;
    vga_set_cursor(cur_col, cur_row + TX_ROW);
}

<<<<<<< HEAD
static EKey editor_read_key(void) {
    int e0 = 0;

    for (;;) {
        while ((inb(0x64) & 0x01) == 0);
        int sc = inb(0x60);

        if (sc == SC_E0) {
=======
static EKey editor_read_key(void)
{
    int e0 = 0;

    for (;;)
    {
        while ((inb(0x64) & 0x01) == 0)
            ;
        int sc = inb(0x60);

        if (sc == SC_E0)
        {
>>>>>>> f7fec4a (added new things and bug fixes)
            e0 = 1;
            continue;
        }

<<<<<<< HEAD
        if (sc & 0x80) {
            int make = sc & 0x7F;
            if (make == SC_CTRL) ctrl_pressed = 0;
            else if (make == SC_LSHIFT || make == SC_RSHIFT) shift_pressed = 0;
            continue;
        }

        if (sc == SC_CTRL) { ctrl_pressed = 1; continue; }
        if (sc == SC_LSHIFT || sc == SC_RSHIFT) { shift_pressed = 1; continue; }

        if (e0) {
            e0 = 0;
            switch (sc) {
                case 0x48: return make_key(EKEY_UP, 0);
                case 0x50: return make_key(EKEY_DOWN, 0);
                case 0x4B: return make_key(EKEY_LEFT, 0);
                case 0x4D: return make_key(EKEY_RIGHT, 0);
                case 0x47: return make_key(EKEY_HOME, 0);
                case 0x4F: return make_key(EKEY_END, 0);
                case 0x49: return make_key(EKEY_PGUP, 0);
                case 0x51: return make_key(EKEY_PGDN, 0);
                case 0x53: return make_key(EKEY_DEL, 0);
                default: continue;
            }
        }

        if (ctrl_pressed) {
            if (sc == 0x1F) return make_key(EKEY_CTRL_S, 0);
            if (sc == 0x10) return make_key(EKEY_CTRL_Q, 0);
            if (sc == 0x2E) return make_key(EKEY_CTRL_C, 0);
            if (sc == 0x2F) return make_key(EKEY_CTRL_V, 0);
=======
        if (sc & 0x80)
        {
            int make = sc & 0x7F;
            if (make == SC_CTRL)
                ctrl_pressed = 0;
            else if (make == SC_LSHIFT || make == SC_RSHIFT)
                shift_pressed = 0;
            continue;
        }

        if (sc == SC_CTRL)
        {
            ctrl_pressed = 1;
            continue;
        }
        if (sc == SC_LSHIFT || sc == SC_RSHIFT)
        {
            shift_pressed = 1;
            continue;
        }

        if (e0)
        {
            e0 = 0;
            switch (sc)
            {
            case 0x48:
                return make_key(EKEY_UP, 0);
            case 0x50:
                return make_key(EKEY_DOWN, 0);
            case 0x4B:
                return make_key(EKEY_LEFT, 0);
            case 0x4D:
                return make_key(EKEY_RIGHT, 0);
            case 0x47:
                return make_key(EKEY_HOME, 0);
            case 0x4F:
                return make_key(EKEY_END, 0);
            case 0x49:
                return make_key(EKEY_PGUP, 0);
            case 0x51:
                return make_key(EKEY_PGDN, 0);
            case 0x53:
                return make_key(EKEY_DEL, 0);
            default:
                continue;
            }
        }

        if (ctrl_pressed)
        {
            if (sc == 0x18)
                return make_key(EKEY_CTRL_O, 0);
            if (sc == 0x2D)
                return make_key(EKEY_CTRL_X, 0);
            if (sc == 0x2E)
                return make_key(EKEY_CTRL_C, 0);
            if (sc == 0x2F)
                return make_key(EKEY_CTRL_V, 0);
>>>>>>> f7fec4a (added new things and bug fixes)
            continue;
        }

        char c = sc_map[sc];
<<<<<<< HEAD
        if (c == 0) continue;

        if (shift_pressed) {
            if (c >= 'a' && c <= 'z') c -= 32;
        }

        if (c == '\b') return make_key(EKEY_BACKSPACE, 0);
        if (c == '\n') return make_key(EKEY_ENTER, 0);
        if (c == '\t') return make_key(EKEY_TAB, 0);
=======
        if (c == 0)
            continue;

        if (shift_pressed)
        {
            if (c >= 'a' && c <= 'z')
                c -= 32;
        }

        if (c == '\b')
            return make_key(EKEY_BACKSPACE, 0);
        if (c == '\n')
            return make_key(EKEY_ENTER, 0);
        if (c == '\t')
            return make_key(EKEY_TAB, 0);
>>>>>>> f7fec4a (added new things and bug fixes)

        return make_key(EKEY_CHAR, c);
    }
}

static GapBuffer *active_gb = NULL;

<<<<<<< HEAD
void editor_open(const char *filename) {
    GapBuffer *gb = (GapBuffer *)kmalloc(sizeof(GapBuffer));
    if (!gb) return;

    gb->buf = (char *)kmalloc(EDITOR_BUF_SIZE);
    if (!gb->buf) {
=======
void editor_open(const char *filename)
{
    GapBuffer *gb = (GapBuffer *)kmalloc(sizeof(GapBuffer));
    if (!gb)
        return;

    gb->buf = (char *)kmalloc(EDITOR_BUF_SIZE);
    if (!gb->buf)
    {
>>>>>>> f7fec4a (added new things and bug fixes)
        kfree(gb);
        return;
    }

    gb->size = EDITOR_BUF_SIZE;
    gb->gap_start = 0;
    gb->gap_end = EDITOR_BUF_SIZE;
    gb->cursor_row = 0;
    gb->cursor_col = 0;
    gb->modified = 0;

    int i;
<<<<<<< HEAD
    if (filename && filename[0]) {
=======
    if (filename && filename[0])
    {
>>>>>>> f7fec4a (added new things and bug fixes)
        for (i = 0; filename[i] && i < 63; i++)
            gb->filename[i] = filename[i];
        gb->filename[i] = '\0';

<<<<<<< HEAD
        if (fs_exists(filename)) {
            int len = fs_read(filename, gb->buf, EDITOR_BUF_SIZE);
            if (len > 0) {
=======
        if (fs_exists(filename))
        {
            int len = fs_read(filename, gb->buf, EDITOR_BUF_SIZE);
            if (len > 0)
            {
>>>>>>> f7fec4a (added new things and bug fixes)
                gb->gap_start = len;
                gb->gap_end = EDITOR_BUF_SIZE;
            }
        }
<<<<<<< HEAD
    } else {
=======
    }
    else
    {
>>>>>>> f7fec4a (added new things and bug fixes)
        gb->filename[0] = '\0';
    }

    active_gb = gb;
}

<<<<<<< HEAD
void editor_run(void) {
    GapBuffer *gb = active_gb;
    if (!gb) return;
=======
void editor_run(void)
{
    GapBuffer *gb = active_gb;
    if (!gb)
        return;
>>>>>>> f7fec4a (added new things and bug fixes)

    ctrl_pressed = 0;
    shift_pressed = 0;

    vga_clear();

<<<<<<< HEAD
    for (;;) {
=======
    for (;;)
    {
>>>>>>> f7fec4a (added new things and bug fixes)
        render(gb);

        EKey key = editor_read_key();

<<<<<<< HEAD
        switch (key.type) {
            case EKEY_CHAR:
                gb_insert(gb, key.ch);
                break;
            case EKEY_BACKSPACE:
                gb_delete_back(gb);
                break;
            case EKEY_DEL:
                gb_delete_forward(gb);
                break;
            case EKEY_LEFT:
                gb_move_left(gb);
                break;
            case EKEY_RIGHT:
                gb_move_right(gb);
                break;
            case EKEY_UP:
                gb_move_up(gb);
                break;
            case EKEY_DOWN:
                gb_move_down(gb);
                break;
            case EKEY_HOME:
                gb_home(gb);
                break;
            case EKEY_END:
                gb_end(gb);
                break;
            case EKEY_ENTER:
                gb_newline(gb);
                break;
            case EKEY_TAB:
                gb_insert(gb, '\t');
                break;
            case EKEY_CTRL_S:
                editor_save(gb);
                break;
            case EKEY_CTRL_Q:
                goto editor_done;
            default:
                break;
=======
        switch (key.type)
        {
        case EKEY_CHAR:
            gb_insert(gb, key.ch);
            break;
        case EKEY_BACKSPACE:
            gb_delete_back(gb);
            break;
        case EKEY_DEL:
            gb_delete_forward(gb);
            break;
        case EKEY_LEFT:
            gb_move_left(gb);
            break;
        case EKEY_RIGHT:
            gb_move_right(gb);
            break;
        case EKEY_UP:
            gb_move_up(gb);
            break;
        case EKEY_DOWN:
            gb_move_down(gb);
            break;
        case EKEY_HOME:
            gb_home(gb);
            break;
        case EKEY_END:
            gb_end(gb);
            break;
        case EKEY_ENTER:
            gb_newline(gb);
            break;
        case EKEY_TAB:
            gb_insert(gb, '\t');
            break;
        case EKEY_CTRL_O:
            editor_save(gb);
            break;
        case EKEY_CTRL_X:
            goto editor_done;
        default:
            break;
>>>>>>> f7fec4a (added new things and bug fixes)
        }
    }

editor_done:
    kfree(gb->buf);
    kfree(gb);
    active_gb = NULL;
    vga_clear();
}
