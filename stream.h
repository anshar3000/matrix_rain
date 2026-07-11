#ifndef STREAM_H
#define STREAM_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    int pos;
    short int length;
    short int speed;
} stream;

void   init_tail_lookup(void);
stream draw_stream(stream s, int col_index);
stream new_stream();
bool stream_is_finalized(void* ctx, void* data);

#endif /* STREAM_H */
