CC      = gcc
CFLAGS  = -std=c11 -D_DEFAULT_SOURCE -D_XOPEN_SOURCE=600 -Wall -Wextra -I.
LDFLAGS = -lncursesw -lm
SRCS    = main.c grid.c colors.c stream.c timer.c pool_list.c list.c allocator.c pool.c
OBJS    = $(SRCS:.c=.o)
TARGET  = matrix_rain

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJS) $(TARGET)

debug: CFLAGS += -g -fsanitize=address
debug: LDFLAGS += -fsanitize=address
debug: $(TARGET)
