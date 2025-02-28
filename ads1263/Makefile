CFLAGS = -Wall -Wextra -std=c11
OBJS = ads1263.o hwif.o

.PHONY: all clean
all: $(OBJS)

clean:
	rm -f $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $<