#include "history.h"

static char history[HISTORY_SIZE][HISTORY_LINE_SIZE];
static int history_count = 0;

void history_add(const char *line)
{
    if (history_count < HISTORY_SIZE)
    {
        int i = 0;
        while (line[i] != '\0' && i < HISTORY_LINE_SIZE - 1)
        {
            history[history_count][i] = line[i];
            i++;
        }
        history[history_count][i] = '\0';
        history_count++;
    }
}

const char *history_get(int idx)
{
    if (idx >= 0 && idx < history_count)
    {
        return history[idx];
    }
    return "";
}

int history_get_count(void)
{
    return history_count;
}