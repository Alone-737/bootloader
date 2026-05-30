#include "editor.h"
#include "vga.h"
#include "keyboard.h"
#include "keys.h"
#include "fs.h"
#include "kheap.h"

static volatile uint16_t *vga_buf = (volatile uint16_t *)VGA_MEMORY;

#define VGA_ENTRY(c, color) ((uint16_t)(unsigned char)(c) | ((uint16_t)(color) << 8))
#define ST_COLOR (VGA_COLOR_WHITE | (VGA_COLOR_BLUE << 4))
#define TXT_COLOR (VGA_COLOR_WHITE | (VGA_COLOR_BLACK << 4))
#define ST_ROW 0
#define TX_ROW 1
#define TX_HEIGHT (VGA_HEIGHT - 1)

#define SC_E0 0xE0

static int ctrl_pressed = 0;
static int shift_pressed = 0;

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
    EKeyType type;
    char ch;
} EKey;

static EKey make_key(EKeyType t, char c)
{
    EKey k;
    k.type = t;
    k.ch = c;
    return k;
}

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
        int count = gb->gap_start - pos;
        int i;
        for (i = 0; i < count; i++)
            gb->buf[gb->gap_end - count + i] = gb->buf[pos + i];
        gb->gap_start = pos;
        gb->gap_end -= count;
    }
    else if (pos > gb->gap_start)
    {
        int count = pos - gb->gap_start;
        int i;
        for (i = 0; i < count; i++)
            gb->buf[gb->gap_start + i] = gb->buf[gb->gap_end + i];
        gb->gap_start = pos;
        gb->gap_end += count;
    }
}

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
            col++;
        }
    }
    gb->cursor_row = row;
    gb->cursor_col = col;
}

static void gb_insert(GapBuffer *gb, char c)
{
    if (gb_is_full(gb))
        return;
    gb->buf[gb->gap_start] = c;
    gb->gap_start++;
    gb->modified = 1;
}

static void gb_newline(GapBuffer *gb)
{
    gb_insert(gb, '\n');
}

static void gb_delete_back(GapBuffer *gb)
{
    if (gb->gap_start == 0)
        return;
    gb->gap_start--;
    gb->modified = 1;
}

static void gb_delete_forward(GapBuffer *gb)
{
    if (gb->gap_end >= gb->size)
        return;
    gb->gap_end++;
    gb->modified = 1;
}

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
    int pos = gb->gap_start;
    while (pos > 0 && gb->buf[pos - 1] != '\n')
        pos--;
    gb_move_to(gb, pos);
}

static void gb_end(GapBuffer *gb)
{
    int pos = gb->gap_end;
    while (pos < gb->size && gb->buf[pos] != '\n')
        pos++;
    int log_pos = gb->gap_start + (pos - gb->gap_end);
    gb_move_to(gb, log_pos);
}

static void gb_move_up(GapBuffer *gb)
{
    update_cursor_pos(gb);
    int target_col = gb->cursor_col;

    int cur_line_start = gb->gap_start;
    while (cur_line_start > 0 && gb->buf[cur_line_start - 1] != '\n')
        cur_line_start--;

    if (cur_line_start == 0)
        return;

    int prev_line_end = cur_line_start - 2;
    if (prev_line_end < 0)
    {
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

static void gb_move_down(GapBuffer *gb)
{
    update_cursor_pos(gb);
    int target_col = gb->cursor_col;

    int cur_line_end = gb->gap_end;
    while (cur_line_end < gb->size && gb->buf[cur_line_end] != '\n')
        cur_line_end++;

    if (cur_line_end >= gb->size)
        return;

    int next_line_start = cur_line_end + 1;
    if (next_line_start >= gb->size)
        return;

    int next_line_end = next_line_start;
    while (next_line_end < gb->size && gb->buf[next_line_end] != '\n')
        next_line_end++;

    int next_len = next_line_end - next_line_start;
    if (target_col > next_len)
        target_col = next_len;

    int log_pos = gb->gap_start + (next_line_start - gb->gap_end) + target_col;
    gb_move_to(gb, log_pos);
}

static void editor_save(GapBuffer *gb)
{
    if (gb->filename[0] == '\0')
        return;
    int content_len = gb_logical_size(gb);
    if (content_len > FS_SIZE)
        content_len = FS_SIZE;

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
        buf[p++] = 'u';
        buf[p++] = 'n';
        buf[p++] = 'n';
        buf[p++] = 'a';
        buf[p++] = 'm';
        buf[p++] = 'e';
        buf[p++] = 'd';
    }

    buf[p++] = ' ';
    buf[p++] = '[';
    buf[p++] = gb->modified ? '*' : ' ';
    buf[p++] = ']';
    buf[p++] = ' ';

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

    while (p < 80)
        buf[p++] = ' ';

    uint8_t color = ST_COLOR;
    for (int x = 0; x < VGA_WIDTH; x++)
        vga_buf[x] = VGA_ENTRY(buf[x], color);
}

static void render(GapBuffer *gb)
{
    for (int y = 0; y < VGA_HEIGHT; y++)
        for (int x = 0; x < VGA_WIDTH; x++)
            vga_buf[y * VGA_WIDTH + x] = VGA_ENTRY(' ', TXT_COLOR);

    draw_status_bar(gb);

    int row = 0, col = 0;
    int cur_row = 0, cur_col = 0;
    int at_cursor = 0;

    for (int i = 0; i < gb->size; i++)
    {
        if (i == gb->gap_start)
        {
            at_cursor = 1;
            cur_row = row;
            cur_col = col;
            i = gb->gap_end;
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
            if (col >= VGA_WIDTH)  
            {
                col = 0;
                row++;
            }
            if (col < VGA_WIDTH && row < TX_HEIGHT)
                vga_buf[(row + TX_ROW) * VGA_WIDTH + col] = VGA_ENTRY(gb->buf[i], TXT_COLOR);
            col++;
        }
    }

    if (!at_cursor)
    {
        cur_row = row;
        cur_col = col;
    }

    gb->cursor_row = cur_row;
    gb->cursor_col = cur_col;
    vga_set_cursor(cur_col, cur_row + TX_ROW);
}

static EKey editor_read_key(void)
{
    int e0 = 0;

    for (;;)
    {
        while (!keyboard_data_available())
            ;
        unsigned char sc = keyboard_read_raw();

        if (sc == SC_E0)
        {
            e0 = 1;
            continue;
        }

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
            case SC_E0_UP:
                return make_key(EKEY_UP, 0);
            case SC_E0_DOWN:
                return make_key(EKEY_DOWN, 0);
            case SC_E0_LEFT:
                return make_key(EKEY_LEFT, 0);
            case SC_E0_RIGHT:
                return make_key(EKEY_RIGHT, 0);
            case SC_E0_HOME:
                return make_key(EKEY_HOME, 0);
            case SC_E0_END:
                return make_key(EKEY_END, 0);
            case SC_E0_PGUP:
                return make_key(EKEY_PGUP, 0);
            case SC_E0_PGDN:
                return make_key(EKEY_PGDN, 0);
            case SC_E0_DEL:
                return make_key(EKEY_DEL, 0);
            default:
                continue;
            }
        }

        if (ctrl_pressed)
        {
            if (sc == SC_O)
                return make_key(EKEY_CTRL_O, 0);
            if (sc == SC_X)
                return make_key(EKEY_CTRL_X, 0);
            if (sc == SC_C)
                return make_key(EKEY_CTRL_C, 0);
            if (sc == SC_V)
                return make_key(EKEY_CTRL_V, 0);
            continue;
        }

        char c = scancode_map[sc];
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

        return make_key(EKEY_CHAR, c);
    }
}

static GapBuffer *active_gb = NULL;

void editor_open(const char *filename)
{
    GapBuffer *gb = (GapBuffer *)kmalloc(sizeof(GapBuffer));
    if (!gb)
        return;

    gb->buf = (char *)kmalloc(EDITOR_BUF_SIZE);
    if (!gb->buf)
    {
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
    if (filename && filename[0])
    {
        for (i = 0; filename[i] && i < 63; i++)
            gb->filename[i] = filename[i];
        gb->filename[i] = '\0';

        if (fs_exists(filename))
        {
            int len = fs_read(filename, gb->buf, EDITOR_BUF_SIZE);
            if (len > 0)
            {
                gb->gap_start = len;
                gb->gap_end = EDITOR_BUF_SIZE;
            }
        }
    }
    else
    {
        gb->filename[0] = '\0';
    }

    active_gb = gb;
}

void editor_run(void)
{
    GapBuffer *gb = active_gb;
    if (!gb)
        return;

    ctrl_pressed = 0;
    shift_pressed = 0;

    vga_clear();

    for (;;)
    {
        render(gb);

        EKey key = editor_read_key();

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
        }
    }

editor_done:
    kfree(gb->buf);
    kfree(gb);
    active_gb = NULL;
    vga_clear();
}
