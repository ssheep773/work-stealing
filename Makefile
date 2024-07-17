CC = gcc
CFLAGS = -O2 -Wall -std=c11
LIBS = -lpthread

all: main

OBJS := workstealing-main.o

deps := $(OBJS:%.o=.%.o.d)

main: $(OBJS)
	$(CC) -o $@ $(LDFLAGS) $^

%.o: %.c
	$(CC) -o $@ $(CFLAGS) -c -MMD -MF .$@.d $<

test: main
	@./main

clean:
	rm -f $(OBJS) $(deps) *~ main
	rm -rf *.dSYM

-include $(deps)