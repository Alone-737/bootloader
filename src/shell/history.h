#ifndef HISTORY_H
#define HISTORY_H

#define HISTORY_SIZE 10
#define HISTORY_LINE_SIZE 80

void history_add(const char *line);
const char *history_get(int idx);
int history_get_count(void);

#endif