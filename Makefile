CFLAGS = -g -Wall -std=c11
SRCS = $(wildcard *.c)
OBJS = $(SRCS:.c=.o)

9cc: $(OBJS)
	gcc $(CFLAGS) $^ -o $@

$(OBJS): 9cc.h

test: 9cc
	./test.sh

clean:
	rm -f 9cc *.o *~ tmp*
