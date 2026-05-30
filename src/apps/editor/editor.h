#ifndef EDITOR_H
#define EDITOR_H

#define EDITOR_BUF_SIZE 8192

typedef struct {
    char *buf;
    int gap_start;
    int gap_end;
    int size;
    int cursor_row;
    int cursor_col;
    char filename[64];
    int modified;
} GapBuffer;

void editor_open(const char *filename);
void editor_run(void);

#endif
