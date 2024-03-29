CFLAGS = -g -Wall -std=c11 -static
SRCS = $(wildcard *.c)
OBJS = $(SRCS:.c=.o)

9cc: $(OBJS)
	gcc $(CFLAGS) $^ -o $@

$(OBJS): 9cc.h

test: 9cc
	./9cc --test
	./test.sh

clean:
	rm -f 9cc *.o *~ tmp*

.PHONY: test clean