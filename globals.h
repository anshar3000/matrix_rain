#ifndef GLOBALS_H
#define GLOBALS_H

#define MAX_STREAM_LENGTH 32
#define MIN_STREAM_LENGTH 8
#define MIN_STREAM_SPEED  1
#define MAX_STREAM_SPEED  4

extern char *grid;
extern int  *tail_size_lookup;
extern int   max_row;
extern int   max_col;
extern int   timerfd;

#endif /* GLOBALS_H */
